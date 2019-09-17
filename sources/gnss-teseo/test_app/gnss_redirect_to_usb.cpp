/**
 * @file gnss_redirect_to_usb.cpp
 * @author Baudouin Feildel <baudouin.feildel@st.com>
 * @brief CLI Application used to redirect an NMEA stream from one UART to USB
 */

#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <stdexcept>
#include <iostream>
#include <atomic>
#include <thread>
#include <csignal>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <future>
#include <cstdlib>

#include <string.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <limits.h>

#define EXIT_ARGS_ERROR      255
#define EXIT_NOT_IMPLEMENTED 100
#define EXIT_TTY_NOT_OPENED   50

#define LOG(msg) std::cout << msg << std::endl

#define LOG_ERROR(msg) std::cerr << msg << std::endl

#define LOG_IF_VERBOSE(msg) if(verbose) { \
std::cout << std::this_thread::get_id()   \
          << " " << msg << std::endl;     \
}

#define ASYNC_LOG_IF_VERBOSE(msg) if(verbose) { \
	std::stringstream ss;                       \
	ss << msg;                                  \
	std::async([] (std::string str) -> int {    \
		std::cout << std::this_thread::get_id() \
		          << " " << str << std::endl;   \
		return 0;                               \
	}, ss.str());                               \
}

typedef struct {
char model[15];
int  gpio_reset;
int  host_uart_port;
} board_params_t;

#define BOARD_NUMBERS	(sizeof(board_params)/(sizeof(board_params[0].model) + sizeof(board_params[0].gpio_reset) + sizeof(board_params[0].host_uart_port)))

board_params_t board_params[] = {
  {"STA1078"    ,  19, 3},
  {"STA1085"    , 206, 1},
  {"STA1095"    , 206, 1},
  {"STA1195"    , 101, 1},
  {"STA1295"    , 101, 1},
  {"STA1385"    ,  62, 2},
  {"STA1385 MTP",  62, 2},
};

#define SZ 255
char AppliName[40];
char BinaryPath[PATH_MAX]="/usr/bin/";
bool TeseoReset_flag = true;
int  GpioTeseoReset = -1;
int  Host_uart_port = -1;
char Uart2Host[20] = "/dev/ttyAMA";
bool Display_flag = false;


namespace internal {

	void report_error(const char * msg, int errno_value, bool nothrow = false)
	{
		auto errno_description = strerror(errno_value);
		LOG_ERROR(msg << ", error #" << errno_value << ": " << errno_description);

		if(!nothrow)
			throw std::runtime_error(msg);
	}

	void report_error(const std::string & msg, int errno_value, bool nothrow = false)
	{
		report_error(msg.c_str(), errno_value, nothrow);
	}

	void report_error(const std::stringstream & msg, int errno_value, bool nothrow = false)
	{
		report_error(msg.str().c_str(), errno_value, nothrow);
	}

}

class usb {
public:
	enum class mode {
		unknown,
		device,
		host
	};
	
	enum class gadget_class {
		unknown,
		ncm,
		acm,
		ecm,
		mass_storage,
		rndis,
		multi
	};

	void set_mode(mode m)
	{
		if(m != mode::unknown && m != this->m)
		{
			this->m = m;
			this->phy_select_device();
			this->phy_set_mode();
		}
	}

	void set_gadget_class(gadget_class gc)
	{
		if(gc != this->gc)
		{
			disable_current_gadget();
			this->gc = gc;
			enable_current_gadget();
		}
	}

	static const char * gadget_class_to_string(gadget_class gc)
	{
		switch(gc)
		{
			case gadget_class::acm:          return "acm";
			case gadget_class::ecm:          return "ecm";
			case gadget_class::mass_storage: return "mass_storage";
			case gadget_class::multi:        return "multi";
			case gadget_class::ncm:          return "ncm";
			case gadget_class::rndis:        return "rndis";

			default:
			case gadget_class::unknown: return "unknown";
		}
	}

	static const char * mode_to_string(mode m)
	{
		switch(m)
		{
			case mode::device: return "DEVICE";
			case mode::host:   return "HOST";
			default:
			case mode::unknown: return "UNKNOWN";
		}
	}

	const std::string dev_name;
	const std::string udc_name;
	const std::string phy_sys_dir;
	const int phy_device_id;

private:
	constexpr static char gadget_configure_cmd[] = "/bin/bash /usr/bin/a5_usb_gadget.sh -d ";

	mode m;
	gadget_class gc;

	void disable_current_gadget()
	{
		if(gc == gadget_class::unknown)
			return;

		std::stringstream ss;

		ss << gadget_configure_cmd
		   << udc_name << " "
		   << gadget_class_to_string(gc) << " disable";

		// Execute command
		LOG("Execute command: " << ss.str());
		int ret = system(ss.str().c_str());

		if(ret == 0)
			LOG("Command executed correctly." << std::endl);
		else
			LOG_ERROR("Error while executing command, returned value: "
			          << ret << std::endl);
	}

	void enable_current_gadget()
	{
		if(gc == gadget_class::unknown)
			return;

		std::stringstream ss;

		ss << gadget_configure_cmd
		   << udc_name << " "
		   << gadget_class_to_string(gc) << " enable";

		// Execute command
		LOG("Execute command: " << ss.str());
		int ret = system(ss.str().c_str());

		if(ret == 0)
			LOG("Command executed correctly." << std::endl);
		else
			LOG_ERROR("Error while executing command, returned value: "
			          << ret << std::endl);
	}

	void phy_select_device()
	{
		const static std::string selector = phy_sys_dir + std::string("usb_id");
		const char dev_id[2] = {static_cast<char>(phy_device_id + '0'), '\0'};

		// Write device id to selector file
		std::fstream selector_stream(selector);
		selector_stream.write(dev_id, 2);
	}

	void phy_set_mode()
	{
		const static std::string selector = phy_sys_dir + std::string("usb_mode");

		if(m == mode::unknown)
			return;

		// Write device mode to selector file
		std::fstream selector_stream(selector);
		selector_stream.write(mode_to_string(m), m == mode::device ? 7 : 5);
	}

	usb(const char * dev_name,
		const char * udc_name,
		const char * phy_sys_dir,
		int phy_device_id) :
		dev_name(dev_name),
		udc_name(udc_name),
		phy_sys_dir(phy_sys_dir),
		phy_device_id(phy_device_id),
		m(mode::unknown),
		gc(gadget_class::unknown)
	{ }

public:
	static usb usb0;
	static usb usb1;
};

constexpr char usb::gadget_configure_cmd[];
usb usb::usb0("USB0", "48400000.usb", "/sys/devices/platform/soc/48440000.usb-phy/", 0);
usb usb::usb1("USB1", "48500000.usb", "/sys/devices/platform/soc/48440000.usb-phy/", 1);

class tty {
private:
	int fd;

public:
	enum class status {
		ready_to_read,
		ready_to_write,
		error,
		hangup,
		not_opened,
		not_ready
	};

	constexpr static std::size_t BUFFER_SIZE = 255;

	using buffer = std::array<uint8_t, BUFFER_SIZE>;

	using var_buffer = std::vector<uint8_t>;

	const std::string device;

	tty(const std::string & device_path) :
		fd(-1),
		device(device_path)
	{ }

	~tty()
	{
		if(opened())
		{
			close();
		}
	}

	void open()
	{
		if(opened())
			return;

		fd = ::open(device.c_str(), O_RDWR | O_NOCTTY);

		if(fd == -1)
		{
			auto errno_cpy = errno;
			internal::report_error(
				std::string("Unable to open tty: ") + device,
				errno_cpy);
		}

		struct termios attr;
		tcgetattr(fd, &attr);

		// Set input/output baudrate
		cfsetispeed(&attr, B115200);
		cfsetospeed(&attr, B115200);

		// Disable stream modifications by kernel
		cfmakeraw(&attr);

		// CREAD => enable receiver
		// CLOCAL => local mode
		attr.c_cflag |= (CLOCAL | CREAD);

		// Apply new attributes
		tcsetattr(fd, TCSANOW, &attr);

		if (tcsetattr(fd,TCSANOW,&attr))
		{
			auto errno_cpy = errno;
			internal::report_error(
				std::string("Unable to set tty attributes: ") + device,
				errno_cpy);
		}

		if (tcflush(fd, TCIOFLUSH))
		{
			auto errno_cpy = errno;
			internal::report_error(
				std::string("Unable to flush tty: ") + device,
				errno_cpy);
		}
	}

	void close()
	{
		if(opened())
		{
			::close(fd);
			fd = -1;
		}
	}

	bool opened() const
	{
		return fd != -1;
	}

	status wait_for_read()
	{
		struct pollfd pfd;
		pfd.fd = fd;
		pfd.events = POLLIN;

		int ret = poll(&pfd, 1, 100);

		if(ret == 0)
			return status::not_ready;

		if(pfd.revents & POLLERR)
			return status::error;

		if(pfd.revents & POLLHUP)
			return status::hangup;

		if(pfd.revents & POLLNVAL)
			return status::not_opened;

		if(pfd.revents & POLLIN)
			return status::ready_to_read;

		return status::error;
	}
	
	status wait_for_write()
	{
		struct pollfd pfd;
		pfd.fd = fd;
		pfd.events = POLLOUT;

		int ret = poll(&pfd, 1, 100);

		if(ret == 0)
			return status::not_ready;

		if(pfd.revents & POLLERR)
			return status::error;

		if(pfd.revents & POLLHUP)
			return status::hangup;

		if(pfd.revents & POLLNVAL)
			return status::not_opened;

		if(pfd.revents & POLLOUT)
			return status::ready_to_write;

		return status::error;
	}

	std::pair<buffer, int> read()
	{
		if(opened())
		{
			buffer b;

			int ret = ::read(fd, b.data(), BUFFER_SIZE);
			if(ret == -1)
			{
				auto errno_cpy = errno;
				internal::report_error(
					std::string("Error while reading from tty: ") + device,
					errno_cpy);
			}
			else
			{
			  if (Display_flag == true)
			    LOG("" << b.data());
			}

			return std::make_pair(b, ret);
		}
		else
		{
			throw std::runtime_error("tty isn't opened, can't read.");
		}
	}

	template<typename Tbuffer>
	auto write(const std::pair<Tbuffer, int> & b)
	{
		if(opened())
		{
			auto ret = ::write(fd, b.first.data(), b.second);
			if(ret == -1)
			{
				auto errno_cpy = errno;
				internal::report_error(
					std::string("Error while writing to tty: ") + device,
					errno_cpy);
			}

			return ret;
		}
		else
		{
			throw std::runtime_error("tty isn't opened, can't write.");
		}
	}
};

std::condition_variable wait_for_interrupt;
std::mutex wfi_mutex;

enum class interrupt_type {
	none,
	exit_request,
	redirection_error
};

interrupt_type interrupt_reason = interrupt_type::none;

void tty_redirect(tty & source, tty & destination, std::atomic_bool & stop, bool verbose, std::string name)
{
	std::string verbose_log_prepend;
	{
		std::stringstream ss;
		ss << name << source.device << " -> " << destination.device << " : transfer ";
		verbose_log_prepend = ss.str();
	}

	while(!stop.load() && source.opened() && destination.opened())
	{
		try
		{
			/*
			LOG_IF_VERBOSE(name << " Wait for " << source.device << " to be available for read.");
			while(source.wait_for_read() != tty::status::ready_to_read && !stop.load() && source.opened());
			if(stop)
			{
				LOG_IF_VERBOSE(name << " Stop requested, exit.");
				break;
			}
			//*/

			//auto buffer = ;
			//LOG_IF_VERBOSE(name << " Readed " << buffer.second << " bytes from " << source.device);

			/*
			LOG_IF_VERBOSE(name << " Wait for " << destination.device << " to be available for write.");
			while(destination.wait_for_write() != tty::status::ready_to_write && !stop.load() && destination.opened());
			if(stop)
			{
				LOG_IF_VERBOSE(name << " Stop requested, exit.");
				break;
			}
			//*/

			auto count = destination.write(source.read());
			ASYNC_LOG_IF_VERBOSE(verbose_log_prepend << count << " bytes.");
		}
		catch(const std::runtime_error & ex)
		{
			/*
			 * If stop was requested exceptions are expected (uarts are closed)
			 * Then the function will exit.
			 * 
			 * Otherwise we display a message and let the function exits by
			 * setting stop to true.
			 */
			if(!stop)
			{
				LOG_ERROR(name << " Error while redirecting "
				          << source.device << " to " << destination.device
						  << std::endl
						  << "What: " << ex.what());
				stop = true;
			}
		}
	}

	// Request exit of program if not already done.
	if(interrupt_reason == interrupt_type::none)
	{
		LOG_IF_VERBOSE(name << " Interrupt reason is set to none, set it to error and notify main thread.");
		interrupt_reason = interrupt_type::redirection_error;
		wait_for_interrupt.notify_all();
	}

	LOG_IF_VERBOSE(name << " End of thread.");
}

void stop_teseo()
{
  char CmdLine[50];

  printf("\r\n\n***** Quit USB redirection *****\r\n");
  if (TeseoReset_flag == true) {
    sprintf(CmdLine, "echo 0 > /sys/class/gpio/gpio%d/value", GpioTeseoReset); system(CmdLine);
  }
  sprintf(CmdLine, "killall -9 %s", AppliName); system(CmdLine);
}

void signal_handler(int sig)
{
	switch(sig)
	{
	case SIGTERM:
		LOG("SIGTERM received, exiting." << std::endl);
		break;

	case SIGINT:
		LOG("SIGINT received, exiting." << std::endl);
        stop_teseo();
        exit(0);
		break;

	case SIGABRT:
		LOG("SIGABRT received, exiting." << std::endl);
		break;
	}

	interrupt_reason = interrupt_type::exit_request;
	wait_for_interrupt.notify_all();
}

struct options {
	std::string teseo_uart_name;
	std::string usb_uart_name;
	usb * usb_output;
	bool help;
	bool error;
	bool verbose;
};

void app_help(const char * bin_name)
{
	LOG("" << std::endl);
	LOG(bin_name << " help"                                                    << std::endl
		<< "This program allows you to redirect the Teseo UART to an USB port" << std::endl
		<< "of the board. The USB will be visible as a serial port from a PC." << std::endl
		<< std::endl
		<< "Options:"                                                          << std::endl
		<< "	IN   x"                                                        << std::endl
		<< "		User can specify the Host<->Teseo UART port x of the host" << std::endl
		<< std::endl
		<< "	GPIO z"                                                        << std::endl
		<< "		User can specify the GPIO z to reset the Teseo"            << std::endl
		<< std::endl
		<< "	-o usb# | --usb-output usb#"                                   << std::endl
		<< "		Set the USB output to usb0 or usb1"                        << std::endl
		<< std::endl
		<< "	-u /dev/ttyGS0 | --usb-tty /dev/ttyGS0"                        << std::endl
		<< "		Set the USB UART device (default is /dev/ttyGS0)"          << std::endl 
		<< "		Change only if another usb port is used as serial device"  << std::endl
		<< std::endl
		<< "	-no_reset"                                                     << std::endl
		<< "		No Teseo reset performed at start/stop"                    << std::endl
		<< std::endl
		<< "	-h | --help"                                                   << std::endl
		<< "		Display the \"Usage\" message"                             << std::endl
		<< std::endl
		<< "	-d | --display"                                                << std::endl
		<< "		Enable the trace on the STDOUT"                            << std::endl
		<< std::endl
		<< "	-v | --verbose"                                                << std::endl
		<< "		Enable verbose output"                                     << std::endl
	);
}


void get_appli_name(const char *input)
{
  if (strrchr(input, '/') != NULL)
	strcpy(AppliName, strrchr(input, '/') + 1);
  else
	strcpy(AppliName, input);
}

void get_appli_path(const char *pathname)
{
  char path_save[PATH_MAX];
  char *p;

  strcpy(path_save, pathname);
  p = strrchr(path_save, '/');
  if( p != NULL )
  {
    path_save[p-path_save+1] = '\0';
    strcpy(BinaryPath, path_save);
  }

  printf("Absolute path to executable is: %s\n", BinaryPath);
}

static int get_board_model(char *model)
{
  FILE *fp;

  fp = fopen("/proc/device-tree/model", "r");
  if (!fp)
    return -ENODEV;

  fgets(model, SZ, fp);
  fclose(fp);

  return 0;
}

void get_default_params(void)
{
  int ret, id;
  char model[SZ];

  ret = get_board_model(model);
  if (ret) {
    printf("Standalone read : Failed to retrieve board model!!\r\n");
    return;
  }
  else {
    printf("Detected board model : \"%s\"\r\n\n", model);
  }

  for (id = 0; id<BOARD_NUMBERS ; id++)
  {
    if (strstr(model, board_params[id].model)) {
      GpioTeseoReset = board_params[id].gpio_reset;
      Host_uart_port = board_params[id].host_uart_port;
    }
  }
}

options get_options(int argc, const char * argv[])
{
	options opts = { "/dev/ttyAMA", "/dev/ttyGS0", &usb::usb0, false, false, false};

	int i = 1;
	while(i < argc)
	{
		std::string arg(argv[i]);

		if(arg == "-h" || arg == "--help")
			opts.help = true;

		else if(arg == "-v" || arg == "--verbose")
			opts.verbose = true;

		else if(arg == "-d" || arg == "--display")
			Display_flag = true;

		else if(arg == "-o" || arg == "--usb-output")
		{
			i++;

			const std::string val(argv[i]);

			if(val == "0" || val == "usb0" || val == "USB0")
				opts.usb_output = &usb::usb0;
			else if(val == "1" || val == "usb1" || val == "USB1")
				opts.usb_output = &usb::usb1;
			else
			{
				LOG_ERROR("Invalid value '" << val << "' for option " << arg);
				opts.error = true;
			}
		}

		else if(arg == "-u" || arg == "--usb-tty")
			opts.usb_uart_name = std::string(argv[++i]);

		else if(arg == "IN")
			Host_uart_port = (int) strtol(argv[++i], (char **)NULL, 10);

		else if(arg == "GPIO")
			GpioTeseoReset = (int) strtol(argv[++i], (char **)NULL, 10);

		else if(arg == "-no_reset")
			TeseoReset_flag = false;

		else
		{
			LOG_ERROR("Invalid argument '" << arg << "'.");
			opts.error = true;
		}

		i++;
	}

	sprintf(Uart2Host ,"%s%d", Uart2Host , Host_uart_port);
	opts.teseo_uart_name = Uart2Host;

	return opts;
}

struct app {
	usb & usb_serial;
	tty teseo_uart;
	tty usb_uart;

	std::thread rx;
	std::thread tx;

	std::atomic_bool stop_rx;
	std::atomic_bool stop_tx;

	bool verbose;

	void teseo_start()
	{
		char CmdLine[50];
		char GpioPathName[40];

		/* ################ Start Teseo in Standalone mode  #################### */
		if (TeseoReset_flag == true) {
		  sprintf(GpioPathName,"/sys/class/gpio/gpio%d",GpioTeseoReset);
		  if( access( GpioPathName, F_OK ) == -1 ) {
		     LOG("Configure new GPIO " << GpioPathName);
		     sprintf(CmdLine, "echo %d > /sys/class/gpio/export", GpioTeseoReset);  system(CmdLine);
		     sprintf(CmdLine, "echo out > %s/direction", GpioPathName); system(CmdLine);
		     sleep(1);
		  }
		  printf("Teseo Reset with gpio %d\r\n",GpioTeseoReset);
		  sprintf(CmdLine, "echo 0 > %s/value", GpioPathName); system(CmdLine);
		  sprintf(CmdLine, "echo 1 > %s/value", GpioPathName); system(CmdLine);
		}
	}

	void usb_enable()
	{
		LOG("Switch " << usb_serial.dev_name << " to device mode and gadget acm class.");
		usb_serial.set_gadget_class(usb::gadget_class::acm);
		usb_serial.set_mode(usb::mode::device);
	}

	void usb_disable()
	{
		LOG("Reset " << usb_serial.dev_name);
		usb_serial.set_mode(usb::mode::host);
		usb_serial.set_gadget_class(usb::gadget_class::unknown);
	}

	bool ttys_open()
	{
		try
		{
			LOG("Open ttys: " << std::endl
					<< "- " << teseo_uart.device << std::endl
					<< "- " << usb_uart.device);

			teseo_uart.open();
			usb_uart.open();
			return true;
		}
		catch(const std::exception & ex)
		{
			LOG_ERROR("Error while opening ttys, exiting");
			LOG_ERROR(ex.what() << std::endl);
			return false;
		}
	}

	void start_redirection()
	{
		LOG_IF_VERBOSE("Set stop flags to false");

		stop_rx = false;
		stop_tx = false;

		LOG_IF_VERBOSE("Start rx & tx threads");

		rx = std::thread(tty_redirect, std::ref(usb_uart), std::ref(teseo_uart), std::ref(stop_rx), verbose, "RX <- ");
		tx = std::thread(tty_redirect, std::ref(teseo_uart), std::ref(usb_uart), std::ref(stop_tx), verbose, "TX -> ");
	}

	void stop_redirection_and_wait()
	{
		LOG_IF_VERBOSE("Set stop flags to true");

		stop_rx = true;
		stop_tx = true;

		LOG_IF_VERBOSE("Wait 100ms");
		usleep(100000);

		LOG_IF_VERBOSE("Close ttys");
		usb_uart.close();
		teseo_uart.close();

		LOG_IF_VERBOSE("Join rx & tx threads");
		rx.join();
		tx.join();
	}

	void wfi()
	{
		LOG_IF_VERBOSE("Wait for interrupt");
		std::unique_lock<std::mutex> lock(wfi_mutex);
		wait_for_interrupt.wait(lock, [] () { return interrupt_reason != interrupt_type::none; });
	}

	app(const options & opts) :
		usb_serial(*opts.usb_output),
		teseo_uart(opts.teseo_uart_name),
		usb_uart(opts.usb_uart_name),
		verbose(opts.verbose)
	{ }

};

int main(int argc, const char * argv[])
{
    printf("\r\n");
    get_appli_path(argv[0]);
    get_appli_name(argv[0]);
    get_default_params();

	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGABRT, signal_handler);

	options opts = get_options(argc, argv);

	if(opts.help || opts.error)
	{
		app_help(argv[0]);
		return opts.error ? EXIT_ARGS_ERROR : EXIT_SUCCESS;
	}

	printf("UART%d : Host <--> Teseo \r\n", Host_uart_port);

	app a(opts);

	while(interrupt_reason != interrupt_type::exit_request)
	{
		interrupt_reason = interrupt_type::none;

		// Reset Teseo if enabled
		a.teseo_start();

		// Switch usb to device mode and gadget acm class
		a.usb_enable();

		// Open Teseo and USB gadget TTYs
		if(!a.ttys_open())
			return EXIT_TTY_NOT_OPENED;

		// Start rx and tx threads and wait for interrupt
		LOG("Launch redirection tasks.");
		a.start_redirection();
		a.wfi();

		LOG("Stopping redirection tasks...");

		a.stop_redirection_and_wait();
		
		LOG("Tasks stopped correctly.");

		a.usb_disable();
	}

	if(opts.verbose)
		LOG("End of application");

	return EXIT_SUCCESS;
}
