package etalLog2Cmd;
use strict;
use warnings;

=head1 NAME

etalLog2Cmd package

=head1 SYNOPSIS

use etalLog2Cmd;

=head1 DESCRIPTION

Package containing function to convert Etal Log into Etal Command script

=cut

#List script function that can be exported
use Exporter;
our @ISA = qw(Exporter);
our @EXPORT = qw( &etalLod2Cmd_convert );


my ($fhin, $fhout, $etalCmd, $paramIdx, $param, $param2, $param3, $param4, $param5, $i, $hReceiverIdx, $hDatapathIdx, $hMonitorIdx, $lineNb, $timestamp, $timestamp_prev);
my (%hEtalCmdList, @hReceivers, @hDatapaths, @hMonitors);

use constant {
	PARAMTYPE_NONE      => 0,
	PARAMTYPE_NUMBER    => 1,
	PARAMTYPE_HRECEIVER => 2,
	PARAMTYPE_HDATAPATH	=> 3,
	PARAMTYPE_HMONITOR => 4
};

use constant ETALDEMO_NB_TUNER_MAX              => 2;
use constant ETALDEMO_NB_CHANNEL_PER_TUNER      => 2;
use constant ETALDEMO_NB_TUNER_CHANNEL_MAX      => (ETALDEMO_NB_TUNER_MAX * ETALDEMO_NB_CHANNEL_PER_TUNER);
use constant ETALDEMO_NB_DATAPATH_MAX           => 10;
use constant ETAL_CAPA_MAX_FRONTEND_PER_TUNER   => 2;
use constant ETAL_CAPA_MAX_TUNER                => 2;
use constant ETAL_CAPA_MAX_FRONTEND             => (ETAL_CAPA_MAX_FRONTEND_PER_TUNER * ETAL_CAPA_MAX_TUNER);

# EtalAPI key => [ EtalCmd, min params, max params ]
%hEtalCmdList = (
#	"etal_initialize"                => [ "ei",     4,  4, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_reinitialize"              => [ "er",     0,  0, PARAMTYPE_NONE],
	"etal_deinitialize"              => [ "ed",     0,  0, PARAMTYPE_NONE],
	"etal_config_receiver"           => [ "ecr",    7,  7, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_destroy_receiver"          => [ "edr",    1,  1, PARAMTYPE_HRECEIVER],
	"etal_config_datapath"           => [ "ecd",    3,  3, PARAMTYPE_HDATAPATH, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etal_destroy_datapath"          => [ "edd",    1,  1, PARAMTYPE_HDATAPATH],
	"etal_config_audio_path"         => [ "ecap",   5,  5, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_audio_select"              => [ "eas",    2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etal_force_mono"                => [ "efm",    2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etal_mute"                      => [ "em",     2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etal_event_FM_stereo_start"     => [ "eeFssta",1,  1, PARAMTYPE_HRECEIVER],
	"etal_event_FM_stereo_stop"      => [ "eeFssto",1,  1, PARAMTYPE_HRECEIVER],
	"etal_debug_config_audio_alignment" => ["eedcaa",1, 1, PARAMTYPE_NUMBER],
	"etal_receiver_alive"            => [ "era",    1,  1, PARAMTYPE_HRECEIVER],
	"etal_xtal_alignment"            => [ "exa",    1,  1, PARAMTYPE_HRECEIVER],
	"etal_get_version"               => [ "egv",    0,  0, PARAMTYPE_NONE],
	"etal_get_capabilities"          => [ "egc",    0,  0, PARAMTYPE_NONE],
	"etal_get_init_status"           => [ "egis",   0,  0, PARAMTYPE_NONE],
	"etal_tune_receiver"             => [ "etr",    2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etal_tune_receiver_async"       => [ "etra",   2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etal_change_band_receiver"      => [ "ecbr",   2,  5, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_get_current_ensemble"      => [ "egce",   1,  1, PARAMTYPE_NUMBER],
	"etal_get_ensemble_list",        => [ "egel",   0,  0, PARAMTYPE_NONE],
	"etal_get_ensemble_data",        => [ "eged",   1,  1, PARAMTYPE_NUMBER],
	"etal_get_fic",                  => [ "egf",    1,  1, PARAMTYPE_HRECEIVER],
	"etal_get_service_list"          => [ "egsl",   4,  4, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_get_specific_service_data_DAB", => [ "egssdD", 2, 2, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_service_select_audio"      => [ "essa",   4,  6, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_service_select_data"       => [ "essd",   4,  6, PARAMTYPE_HDATAPATH, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_get_reception_quality"     => [ "egrq",   1,  1, PARAMTYPE_HRECEIVER],
	"etal_get_channel_quality"       => [ "egcq",   1,  1, PARAMTYPE_HRECEIVER],
	"etal_config_reception_quality_monitor" => [ "ecrqm", 67, 67, PARAMTYPE_HMONITOR, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_destroy_reception_quality_monitor" => [ "edrqm", 1, 1, PARAMTYPE_HMONITOR],
	"etal_get_receiver_frequency"    => [ "egrf",   1,  1, PARAMTYPE_HRECEIVER],
	"etal_get_CF_data"               => [ "egCd",   3,  3, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_seek_start_manual"         => [ "esstam", 3,  3, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_seek_continue_manual"      => [ "escm",   1,  1, PARAMTYPE_HRECEIVER],
	"etal_seek_stop_manual"          => [ "esstom", 2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etal_seek_get_status_manual"    => [ "esgsm",  1,  1, PARAMTYPE_HRECEIVER],
	"etal_autoseek_start"            => [ "essta",  6,  6, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_autoseek_stop"             => [ "essto",  2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etal_seek_stop"                 => [ "essto",  2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER], # line added because of etal wrong printf for etal_autoseek_stop
	"etal_set_autoseek_thresholds_value" => ["esstv",8,  8, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_AF_switch"                 => [ "eAsw",   2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etal_AF_check"                  => [ "eAc",    3,  3, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_AF_start"                  => [ "eAst",   4,  4, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_AF_end"                    => [ "eAe",    2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etal_AF_search_manual"          => [ "eAsm",   3,  3, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_enable_data_service"       => [ "eeds",   7,  7, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_disable_data_service"      => [ "edds",   2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etal_setup_PSD",                => [ "esP",   14, 14, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_read_parameter"            => [ "erp",    3, 12, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_write_parameter"           => [ "ewp",    3, 12, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_start_RDS"                 => [ "estaR",  5,  5, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_stop_RDS"                  => [ "estoR",  1,  1, PARAMTYPE_HRECEIVER],
	"etaltml_get_textinfo"           => [ "egt",    1,  1, PARAMTYPE_HRECEIVER],
	"etaltml_start_textinfo"         => [ "estat",  1,  1, PARAMTYPE_HRECEIVER],
	"etaltml_stop_textinfo"          => [ "estot",  1,  1, PARAMTYPE_HRECEIVER],
	"etaltml_get_decoded_RDS",       => [ "egdR",   1,  1, PARAMTYPE_HRECEIVER],
	"etaltml_start_decoded_RDS",     => [ "estadR", 2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_HRECEIVER],
	"etaltml_stop_decoded_RDS",      => [ "estodR", 2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_HRECEIVER],
	"etaltml_get_validated_RDS_block_manual"=>["egvRbm",1,1,PARAMTYPE_HRECEIVER],
	"etaltml_RDS_AF",                => [ "eRA",    3,  3, PARAMTYPE_HRECEIVER, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etaltml_RDS_TA",                => [ "eTA",    2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etaltml_RDS_EON",               => [ "eRE",    2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etaltml_RDS_REG",               => [ "eRR",    2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etaltml_RDS_AFSearch_start",    => [ "eRAsta", 2, 28, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etaltml_RDS_AFSearch_stop",     => [ "eRAsto", 1,  1, PARAMTYPE_HRECEIVER],
	"etaltml_scan_start",            => [ "escsta", 4,  4, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etaltml_scan_stop",             => [ "escsto", 2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etaltml_learn_start",           => [ "elsta",  4,  4, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etaltml_learn_stop",            => [ "elsto",  2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etaltml_getFreeReceiverForPath" => [ "egFRFP", 2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etaltml_TuneOnServiceId",       => [ "eTOSI",  2,  2, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etaltml_ActivateServiceFollowing"=>[ "eASF",   0,  0, PARAMTYPE_NONE],
	"etaltml_DisableServiceFollowing"=> [ "eDSF",   0,  0, PARAMTYPE_NONE],
	"ETALTML_ServiceFollowing_SeamlessEstimationRequest"=>["eSFSER",0,0,PARAMTYPE_NONE],
	"ETALTML_ServiceFollowing_SeamlessEstimationStop"=>["eSFSES",0,0,PARAMTYPE_NONE],
	"ETALTML_ServiceFollowing_SeamlessSwitchRequest"=>["eSFSSR",1,1,PARAMTYPE_NONE],
	"etal_seamless_estimation_start" => [ "esesta", 2,  5, PARAMTYPE_HRECEIVER, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_seamless_estimation_stop"  => [ "esesto", 2,  2, PARAMTYPE_HRECEIVER, PARAMTYPE_HRECEIVER],
	"etal_seamless_switching"        => [ "essw",   2, 10, PARAMTYPE_HRECEIVER, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_debug_DISS_control"        => [ "edDc",   4,  4, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER],
	"etal_debug_get_WSP_Status"      => [ "edgWS",  1,  1, PARAMTYPE_HRECEIVER],
	"etal_debug_VPA_control"         => [ "edVc",   3,  3, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER],
	"etal_trace_config"              => [ "etc",    5, 29, PARAMTYPE_HRECEIVER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER, PARAMTYPE_NUMBER]
);

# local functions

sub etalLod2Cmd_get_free_hReveiver_Index
{
	my ($i);

	for($i = 0; $i < (ETALDEMO_NB_TUNER_CHANNEL_MAX + 1); $i++)
	{
		if ($hReceivers[$i] == 0)
		{
			last;
		}
	}
	if ($i < (ETALDEMO_NB_TUNER_CHANNEL_MAX + 1))
	{
		return $i;
	}
	else
	{
		return -1;
	}
}

sub etalLod2Cmd_get_hReveiver_Index
{
	my ($hRecv) = @_;
	my ($i);

	for($i = 0; $i < (ETALDEMO_NB_TUNER_CHANNEL_MAX + 1); $i++)
	{
		if ($hReceivers[$i] == $hRecv)
		{
			last;
		}
	}
	if ($i < (ETALDEMO_NB_TUNER_CHANNEL_MAX + 1))
	{
		return $i;
	}
	else
	{
		return -1;
	}
}

sub etalLod2Cmd_add_hReveiver
{
	my ($hRecv) = @_;
	my ($i, $idx);

	for($i = 0; $i < (ETALDEMO_NB_TUNER_CHANNEL_MAX + 1); $i++)
	{
		if ($hReceivers[$i] == $hRecv)
		{
			last;
		}
	}
	if ($i < (ETALDEMO_NB_TUNER_CHANNEL_MAX + 1))
	{
		return $i;
	}
	else
	{
		$idx = etalLod2Cmd_get_free_hReveiver_Index();
		if ($idx != -1)
		{
			$hReceivers[$idx] = $hRecv;
			print $fhout "# add hReceivers[$idx] $hRecv\n\n";
		}
		return $idx;
	}
}

sub etalLod2Cmd_del_hReveiver
{
	my ($hRecv) = @_;
	my ($i, $idx);

	for($i = 0; $i < (ETALDEMO_NB_TUNER_CHANNEL_MAX + 1); $i++)
	{
		if ($hReceivers[$i] == $hRecv)
		{
			last;
		}
	}
	if ($i < (ETALDEMO_NB_TUNER_CHANNEL_MAX + 1))
	{
		$hReceivers[$i] = 0;
		print $fhout "# del hReceivers[$i] $hRecv\n\n";
		return $i;
	}
	else
	{
		return -1;
	}
}

sub etalLod2Cmd_get_free_hDatapath_Index
{
	my ($i);

	for($i = 0; $i < ETALDEMO_NB_DATAPATH_MAX; $i++)
	{
		if ($hDatapaths[$i] == 0)
		{
			last;
		}
	}
	if ($i < ETALDEMO_NB_DATAPATH_MAX)
	{
		return $i;
	}
	else
	{
		return -1;
	}
}

sub etalLod2Cmd_get_hDatapath_Index
{
	my ($hDPath) = @_;
	my ($i);

	for($i = 0; $i < ETALDEMO_NB_DATAPATH_MAX; $i++)
	{
		if ($hDatapaths[$i] == $hDPath)
		{
			last;
		}
	}
	if ($i < ETALDEMO_NB_DATAPATH_MAX)
	{
		return $i;
	}
	else
	{
		return -1;
	}
}

sub etalLod2Cmd_add_hDatapath
{
	my ($hDPath) = @_;
	my ($i, $idx);

	for($i = 0; $i < ETALDEMO_NB_DATAPATH_MAX; $i++)
	{
		if ($hDatapaths[$i] == $hDPath)
		{
			last;
		}
	}
	if ($i < ETALDEMO_NB_DATAPATH_MAX)
	{
		return $i;
	}
	else
	{
		$idx = etalLod2Cmd_get_free_hDatapath_Index();
		if ($idx != -1)
		{
			$hDatapaths[$idx] = $hDPath;
			print $fhout "# add hDatapaths[$idx] $hDPath\n\n";
		}
		return $idx;
	}
}

sub etalLod2Cmd_del_hDatapath
{
	my ($hDPath) = @_;
	my ($i, $idx);

	for($i = 0; $i < ETALDEMO_NB_DATAPATH_MAX; $i++)
	{
		if ($hDatapaths[$i] == $hDPath)
		{
			last;
		}
	}
	if ($i < ETALDEMO_NB_DATAPATH_MAX)
	{
		$hDatapaths[$i] = 0;
		print $fhout "# del hDatapaths[$i] $hDPath\n\n";
		return $i;
	}
	else
	{
		return -1;
	}
}

sub etalLod2Cmd_get_free_hMonitor_Index
{
	my ($i);

	for($i = 0; $i < ETALDEMO_NB_DATAPATH_MAX; $i++)
	{
		if ((!defined($hMonitors[$i])) || ($hMonitors[$i] == 0))
		{
			last;
		}
	}
	if ($i < ETALDEMO_NB_DATAPATH_MAX)
	{
		return $i;
	}
	else
	{
		return -1;
	}
}

sub etalLod2Cmd_get_hMonitor_Index
{
	my ($hMonitor) = @_;
	my ($i);

	for($i = 0; $i < ETALDEMO_NB_DATAPATH_MAX; $i++)
	{
		if ((defined($hMonitors[$i]) && ($hMonitors[$i] == $hMonitor)) ||
			(($hMonitor == 0) && ((!defined($hMonitors[$i])) || ($hMonitors[$i] == 0)) ))
		{
			last;
		}
	}
	if ($i < ETALDEMO_NB_DATAPATH_MAX)
	{
		return $i;
	}
	else
	{
		return -1;
	}
}

sub etalLod2Cmd_add_hMonitor
{
	my ($hMonitor) = @_;
	my ($i, $idx);

	for($i = 0; $i < ETALDEMO_NB_DATAPATH_MAX; $i++)
	{
		if ((defined($hMonitors[$i])) && ($hMonitors[$i] == $hMonitor))
		{
			last;
		}
	}
	if ($i < ETALDEMO_NB_DATAPATH_MAX)
	{
		return $i;
	}
	else
	{
		$idx = etalLod2Cmd_get_free_hMonitor_Index();
		if ($idx != -1)
		{
			$hMonitors[$idx] = $hMonitor;
			print $fhout "# add hMonitors[$idx] $hMonitor\n\n";
		}
		return $idx;
	}
}

sub etalLod2Cmd_del_hMonitor
{
	my ($hMonitor) = @_;
	my ($i, $idx);

	for($i = 0; $i < ETALDEMO_NB_DATAPATH_MAX; $i++)
	{
		if ($hMonitors[$i] == $hMonitor)
		{
			last;
		}
	}
	if ($i < ETALDEMO_NB_DATAPATH_MAX)
	{
		$hMonitors[$i] = 0;
		print $fhout "# del hMonitors[$i] $hMonitor\n\n";
		return $i;
	}
	else
	{
		return -1;
	}
}

sub etalLod2Cmd_get_numberFromString
{
	my ($val) = @_;

	if ($val =~ /0x/)
	{
		# hexadecimal number
		$val =~  s/0x//;
		$val =~  s/\s//g;
		$val = hex($val);
	}
	else
	{
		# decimal number
	}
	return $val;
}

sub etalLod2Cmd_convert
{
	my ($file_input, $file_output) = @_;

	#init
	for($i = 0; $i < (ETALDEMO_NB_TUNER_CHANNEL_MAX + 1); $i++)
	{
		$hReceivers[$i] = 0;
	}
	for($i = 0; $i < ETALDEMO_NB_DATAPATH_MAX; $i++)
	{
		$hDatapaths[$i] = 0;
	}

	$lineNb = 1;
	$timestamp_prev = $timestamp = 0;

	if (open($fhin, "<", $file_input))
	{
		if (!defined($file_output))
		{
			$fhout = <STDOUT>;
			$file_output = "";
		}
		if ((($file_output ne "") && (open($fhout, ">", $file_output))) ||
		    ($file_output eq ""))
		{
			while (<$fhin>)
			{
				# get timestamp
				if (/^\s*(\d+)m\s*,\s*([\d\.]+)\s*s/)
				{
					$timestamp = ($1 * 60) + $2;
				}

				# parse APP to ETAL command
				if (/:\s*APP\s*->\s*ETAL:\s*([^\)\s]+)\s*\(/)
				{
					$etalCmd = $1;
					if (/etal_config_audio_path\s*\(/)
					{
						# remove , res: x
						s/,\s*res\s*:\s*\d+\s*//;
					}
					if (exists $hEtalCmdList{"$etalCmd"})
					{
						if ($timestamp_prev < $timestamp)
						{
							print $fhout "# wait ".(int(($timestamp - $timestamp_prev) * 1000))." ms\n";
							print $fhout "scr_wait: ".(int(($timestamp - $timestamp_prev) * 1000))."\n";
						}
						print $fhout "# $lineNb  $_";
						print $fhout $hEtalCmdList{"$etalCmd"}[0];
						$paramIdx = 0;
						if ($etalCmd eq "etal_config_receiver")
						{
							$_ = <$fhin>; $lineNb++;
							if (/\*rec\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+).*sta\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+).*FroEndSiz\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+).*proFea\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)/)
							{
								$param = etalLod2Cmd_get_numberFromString($1);
								$param2 = etalLod2Cmd_get_numberFromString($2);
								$param3 = etalLod2Cmd_get_numberFromString($3);
								$param4 = etalLod2Cmd_get_numberFromString($4);
								$hReceiverIdx = etalLod2Cmd_get_hReveiver_Index($param);
								print $fhout " $hReceiverIdx $param2";
								$paramIdx  += 2;
								for($i = 0; $i < $param3; $i++)
								{
									$_ = <$fhin>; $lineNb++;
									if (/froEnd\s*\[[^\]]*\]\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)/)
									{
										$param = etalLod2Cmd_get_numberFromString($1);
										print $fhout " $param";
										$paramIdx++;
									}
									else
									{
										print STDERR "etal_config_receiver parameter not found\n";
									}
								}
								if ($i < ETAL_CAPA_MAX_FRONTEND)
								{
									for(; $i < ETAL_CAPA_MAX_FRONTEND; $i++)
									{
										print $fhout " 0";
										$paramIdx++;
									}
								}
								print $fhout " $param4\n";
								$paramIdx++;
							}
							else
							{
								print STDERR "etal_config_receiver parameter not found\n";
							}
						}
						elsif ($etalCmd eq "etal_destroy_receiver")
						{
							if (/-\>\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)/)
							{
								$param = etalLod2Cmd_get_numberFromString($1);
								$hReceiverIdx = etalLod2Cmd_get_hReveiver_Index($param);
								print $fhout " $hReceiverIdx\n";
								$paramIdx++;
								etalLod2Cmd_del_hReveiver($param);
							}
						}
						elsif ($etalCmd eq "etal_destroy_datapath")
						{
							$_ = <$fhin>; $lineNb++;
							if (/datPat\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)/)
							{
								$param = etalLod2Cmd_get_numberFromString($1);
								$hDatapathIdx = etalLod2Cmd_get_hDatapath_Index($param);
								print $fhout " $hDatapathIdx\n";
								$paramIdx++;
								etalLod2Cmd_del_hDatapath($param);
							}
						}
						elsif ($etalCmd eq "etal_config_reception_quality_monitor")
						{
							if (/\(pMon:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)/)
							{
								$hMonitorIdx = etalLod2Cmd_get_hMonitor_Index(etalLod2Cmd_get_numberFromString($1));
								print $fhout " $hMonitorIdx";
								$paramIdx++;
								$_ = <$fhin>; $lineNb++;
								if (/rec:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+),\s*Index\[0\]\s*=>\s*monInd:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+),\s*infVal:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+),\s*supVal:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+),\s*updFreq:*\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)/)
								{
									$param = etalLod2Cmd_get_numberFromString($1);
									$param2 = etalLod2Cmd_get_numberFromString($2);
									$param3 = etalLod2Cmd_get_numberFromString($3);
									$param4 = etalLod2Cmd_get_numberFromString($4);
									$param5 = etalLod2Cmd_get_numberFromString($5);
									$hReceiverIdx = etalLod2Cmd_get_hReveiver_Index($param);
									print $fhout " $hReceiverIdx $param2 $param3 $param4 $param5\n";
									$paramIdx += 5;
								}
							}
						}
						elsif (/etal_destroy_reception_quality_monitor\s*\(\s*pMon\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)/)
						{
							$param = etalLod2Cmd_get_numberFromString($1);
							$hMonitorIdx = etalLod2Cmd_get_hMonitor_Index($param);
							print $fhout " $hMonitorIdx\n";
							$paramIdx++;
							etalLod2Cmd_del_hMonitor($param);
						}
						else
						{
							if ($etalCmd eq "etal_set_autoseek_thresholds_value")
							{
								if (/rec\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)/)
								{
									$param = etalLod2Cmd_get_numberFromString($1);
									$hReceiverIdx = etalLod2Cmd_get_hReveiver_Index($param);
									print $fhout " $hReceiverIdx";
									$paramIdx++;
									$_ = <$fhin>; $lineNb++;
								}
								else
								{
									print STDERR "etal_set_autoseek_thresholds_value parameter not found\n";
								}
							}
							elsif ($etalCmd eq "etal_config_datapath")
							{
								$_ = <$fhin>; $lineNb++;
								if (/datPat\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)/)
								{
									$param = etalLod2Cmd_get_numberFromString($1);
									$hDatapathIdx = etalLod2Cmd_get_hDatapath_Index($param);
									print $fhout " $hDatapathIdx";
									$paramIdx++;
								}
								else
								{
									print STDERR "etal_config_datapath parameter not found\n";
								}
							}
							while((/\(\s*[^\s:]+\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)[\s,:\)]+/))
							{
								$param = $1;
								if ($#{$hEtalCmdList{"$etalCmd"}} >= ($paramIdx + 2))
								{
									if ($hEtalCmdList{"$etalCmd"}[($paramIdx + 3)] == PARAMTYPE_NUMBER)
									{
										print $fhout " ".(etalLod2Cmd_get_numberFromString($param));
										$paramIdx++;
									}
									elsif ($hEtalCmdList{"$etalCmd"}[($paramIdx + 3)] == PARAMTYPE_HRECEIVER)
									{
										print $fhout " ".(etalLod2Cmd_get_hReveiver_Index($param));
										$paramIdx++;
									}
									elsif ($hEtalCmdList{"$etalCmd"}[($paramIdx + 3)] == PARAMTYPE_HDATAPATH)
									{
										print $fhout " ".(etalLod2Cmd_get_hDatapath_Index($param));
										$paramIdx++;
									}
									elsif ($hEtalCmdList{"$etalCmd"}[($paramIdx + 3)] == PARAMTYPE_HMONITOR)
									{
										print $fhout " ".(etalLod2Cmd_get_hMonitor_Index(etalLod2Cmd_get_numberFromString($param)));
										$paramIdx++;
									}
								}
								else
								{
									print STDERR "ERROR unknown parameter $param\n";
								}
								s/(\(\s*)[^\s:]+\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)[\s,:\)]+/$1/;
							}
							print $fhout "\n";
						}
						print $fhout "\n";
						if (($paramIdx < $hEtalCmdList{"$etalCmd"}[1]) || ($paramIdx > $hEtalCmdList{"$etalCmd"}[2]))
						{
							print STDERR "ERROR:$lineNb: parameter number mismatch $etalCmd $paramIdx instead of [".($hEtalCmdList{"$etalCmd"}[1]).", ".($hEtalCmdList{"$etalCmd"}[2])."]\n";
						}
						$timestamp_prev = $timestamp;
					}
					else
					{
						print $fhout "# unknown etal command: $etalCmd\n";
					}
				}
				elsif (/:\s*ETAL\s*->\s*APP:\s*([^\)\s]+)\s*\(/)
				{
					if (/etal_config_receiver\s*\(\s*rec\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)\s*\)\s*=\s*Success/i)
					{
						etalLod2Cmd_add_hReveiver(etalLod2Cmd_get_numberFromString($1));
					}
					elsif (/etal_config_datapath\s*\(\s*datPat\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)\s*\)\s*=\s*Success/i)
					{
						etalLod2Cmd_add_hDatapath(etalLod2Cmd_get_numberFromString($1));
					}
					elsif (/etal_config_reception_quality_monitor\s*\(\s*monHan\s*:\s*(0x\s*[\da-fA-F]+|[-\+\d\.]+)/i)
					{
						etalLod2Cmd_add_hMonitor(etalLod2Cmd_get_numberFromString($1));
					}
				}
				$lineNb++;
			}
			close($fhin);
			if ($file_output ne "")
			{
				close($fhout);
			}
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

1;
