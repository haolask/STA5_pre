#!/usr/bin/perl -w 

use Getopt::Std;
use vars qw($opt_h $opt_n);
use Tk;
use Tk::NoteBook;
use Tk::Dialog;     # for dialog box
use Tk::FBox;       # for FBox file open/save
use Tk::ProgressBar;
use threads;
use threads::shared;
use etal::etal_types;
use etal::etaltml_types;
use etalLog2Cmd;


# variable for GUI widgets
my ($mw, $frameStartQuit, $frameUserCmd, $usercmd, $entryUserCmd, @UserCmdList, $UserCmdIdx);
my ($frameSE, $frameSE2, $frameSS);
my ($etaldemo_started_flag, $esesta_startPos, $esesta_stopPos, $essw_default_flag, $cur_hRecv_idx);
my ($frnbe, $nbe, @nbpe);
my ($frei, $CountryVariantStr, @EtalCountryVariant, %EtalCountryVariant_cnv, $dcop_enable, $tuner1_enable, $tuner2_enable);
my ($fregc, $load_DAB_landscape, $load_AMFM_landscape, $load_HD_landscape);
my ($frecr, $frtecrp, $stdstr, %etal_std_cnv, @etal_std_list, @felist, @etal_fronend_list);
my ($frepfp, $pftfmvpa, $pftantdiv, $pfthdrdrmdigit, $pfthdrblend, $pftall, $pftunspec, $processingFeatures);
my ($frecd);
my ($frecdp, $cur_hdpath_idx, @etal_dpath_list, %etal_dpath_cnv, $dpathtypestr);
my ($frecap, $tuner_idx, $ecap_dac, $ecap_sai_out, $ecap_sai_in, $ecap_sai_slave_mode);
my ($freas, %etal_audio_src_cnv, @etal_audio_src_list, $easrc_select, $forceMonoFlag, $muteFlag);
my ($freeFss, $AutoAlignmentForFM, $AutoAlignmentForAM);
my ($fretr, $frequency);
my ($fretrp, $dab_ch_sel, @dab_ch_list);
my ($frecbr, $bandstr, $fmin, $fmax, @EtalFrequencyBand, %EtalFrequencyBand_cnv);
my ($frecbrp);
my ($frescsta, $audioPlayTime, $scanStart_directionstr, $scanStart_step, $scanStop_terminationMode);
my ($frelsta, $learStart_bandstr, $learStart_step, $learStart_mode, $learnStop_terminationMode);
my ($fregce, $bGetAudioServices, $bGetDataServices);
my ($fressa, $ueid, $mode, @EtalServiceSelectMode, %EtalServiceSelectMode_cnv);
my ($fressap, $service, $sc, $subch);
my ($fressd, $type, @EtalServiceSelectSubFunction, %EtalServiceSelectSubFunction_cnv);
my ($freeds, $freeds1, $freeds2, $freeds3, $freeds31, $freeds32, $ServiceBitmap, @ServiceBitmapList, %ServiceBitmap_cnv, @ServiceBitmapSelected, $freedssb, $freedslb, $logoType);
my (@EtalDataService_EpgLogoType, %EtalDataService_EpgLogoType_cnv);
my ($fresP, $PSDServiceEnableBitmap, @PSDServiceEnableBitmapList, %PSDServiceEnableBitmap_cnv, $fresPsb, $fresPlb);
my ($fresP2, $pConfigLenSet, $PSDTitleLength, $PSDArtistLength, $PSDAlbumLength, $PSDGenreLength, $PSDCommentShortLength, $PSDCommentLength, $PSDUFIDLength);
my ($PSDCommercialPriceLength, $PSDCommercialContactLength, $PSDCommercialSellerLength, $PSDCommercialDescriptionLength, $PSDXHDRLength);
my ($fregrq);
my ($frecrqm, @EtalBcastQaIndicators, %EtalBcastQaIndicators_cnv, $cur_hmonitor_idx, $monitoredContext);
my ($frecrqmp, $monitoredIndicators_idx, @monitoredIndicators, $MonitoredIndicator, $InferiorValue, $SuperiorValue, $UpdateFrequency);
my ($fregCd, $nbOfAverage, $period);
my ($fresstam, @etalSeekDirectionTy, %etalSeekDirectionTy_cnv, $directionstr, $step);
my ($fressstom, $exitSeekAction);
my ($freass, $terminationMode, $seekHDSPS, $updateStopFrequency);
my ($fesstv, $rffs, $bbfs, $detune, $adjacentch, $multipath);
my ($freAs, @etalAFModeTy, %etalAFModeTy_cnv, $AFModestr);
my ($freAsp, $alternateFrequency, $alt_dab_ch_sel, @antennaSelectionTy, %antennaSelectionTy_cnv, $antennaSelectionstr);
my ($freAsm, $AFList, $AFListstr);
my ($fresR, $forceFastPi, $numPi, $rbdsmode, $groupOutEn, $errThresh);
my ($frestat);
my ($frestadR, $RDSServiceList, $RDSServiceList_PS, $RDSServiceList_DI, $RDSServiceList_PI, $RDSServiceList_TOM, $RDSServiceList_RT, $RDSServiceList_AF, $RDSServiceList_PTY, $RDSServiceList_TP, $RDSServiceList_TA, $RDSServiceList_MS, $RDSServiceList_ALL);
my ($fregvRbm, $hReceiverB, $AFOn, $TAOn, $REGOn);
my ($freRE, $EONOn, $RDSAFSearchData_PI, $RDSAFSearchData_AFListStr, $RDSAFSearchData_AFList);
my ($freRAsto, $RDSSeekStart_directionstr, $RDSSeekStart_step, $RDSSeekStart_mute);
my ($fregFRFP, @EtalPathName, %EtalPathName_cnv, $vI_path, $vI_SearchedServiceID);
my ($freSFSER, $freSFSSR, $isSeamless);
my ($freASF);
my ($frerp, $etalReadWriteMode, $param);
my ($fredDc, @etalChannelTy, %etalChannelTy_cnv, $tuner_channel, $DISSmode, $filter_index),
my ($fredVc, $status, $hReceiver_bg, $header, $headerUsed, $defaultLevel);
my ($fredVcp, $defaultLevelUsed, $filterNum, $filterClassStr, $filterMaskStr, $filterLevelStr, $filterClass, $filterMask, $filterLevel);
my ($frescp, $BtnStartScp, $filescp, $scpProgress, $fileEtalLog, $fileEtalLogScp);
my ($frescpfn);
my $scpExecAbortReq:shared = 0;
my $scpExecAbortRsp:shared = 1;
my $scpExecProgress:shared = 0;
my $guiPrintLock:shared = 0;
my ($second_hRecv_idx);

# defines

use constant{
	TAB_INIT_IDX  => 0,
	TAB_CFG_IDX   => 1,
	TAB_DPATH_IDX => 2,
	TAB_AUDIO_IDX => 3,
	TAB_TUNE_IDX  => 4,
	TAB_SVC_IDX   => 5,
	TAB_DATA_IDX  => 6,
	TAB_QUAL_IDX  => 7,
	TAB_SEEK_IDX  => 8,
	TAB_AF_IDX    => 9,
	TAB_RDS_IDX   => 10,
	TAB_SS_IDX    => 11,
	TAB_SF_IDX    => 12,
	TAB_RDWR_IDX  => 13,
	TAB_DBG_IDX   => 14,
	TAB_SCP_IDX   => 15
};

# variable initialization

@EtalCountryVariant = ("UNDEF", "EU", "CHINA", "KOREA");
%EtalCountryVariant_cnv = ("UNDEF" => ETAL_COUNTRY_VARIANT_UNDEF, "EU" => ETAL_COUNTRY_VARIANT_EU, 
	"CHINA" => ETAL_COUNTRY_VARIANT_CHINA, "KOREA" => ETAL_COUNTRY_VARIANT_KOREA);

@etal_std_list = ( "UNDEF", "DAB", "DRM", "FM", "AM", "HD_FM", "HD", "HD_AM" );
%etal_std_cnv = (
	"UNDEF" => ETAL_BCAST_STD_UNDEF, "DAB" => ETAL_BCAST_STD_DAB,
	"DRM" => ETAL_BCAST_STD_DRM,     "FM" => ETAL_BCAST_STD_FM,
	"AM" => ETAL_BCAST_STD_AM,       "HD_FM" => ETAL_BCAST_STD_HD_FM, 
	"HD" => ETAL_BCAST_STD_HD,       "HD_AM" => ETAL_BCAST_STD_HD_AM
);

@etal_fronend_list = (ETAL_FE_HANDLE_1, ETAL_FE_HANDLE_2, ETAL_FE_HANDLE_3, ETAL_FE_HANDLE_4 );

%etal_audio_src_cnv = (
	"src_auto_HD" => ETAL_AUDIO_SOURCE_AUTO_HD,         "src_STAR_AMFM" => ETAL_AUDIO_SOURCE_STAR_AMFM,
	"src_DCOP_STA680" => ETAL_AUDIO_SOURCE_DCOP_STA680, "src_HD_ALIGN" => ETAL_AUDIO_SOURCE_HD_ALIGN,
	"src_DCOP_STA660" => ETAL_AUDIO_SOURCE_DCOP_STA660
);

@etal_dpath_list = ( "UNDEF", "AUDIO", "DCOP AUDIO", "DATA SERVICE", "DAB DATA RAW", "DAB AUDIO RAW", "DAB FIC", "TEXTINFO", "FM RDS", "FM RDS RAW" );
%etal_dpath_cnv = (
	"UNDEF" => ETAL_DATA_TYPE_UNDEF,                "AUDIO" => ETAL_DATA_TYPE_AUDIO,
	"DCOP AUDIO" => ETAL_DATA_TYPE_DCOP_AUDIO,      "DATA SERVICE" => ETAL_DATA_TYPE_DATA_SERVICE,
	"DAB DATA RAW" => ETAL_DATA_TYPE_DAB_DATA_RAW,  "DAB AUDIO RAW" => ETAL_DATA_TYPE_DAB_AUDIO_RAW, 
	"DAB FIC" => ETAL_DATA_TYPE_DAB_FIC,            "TEXTINFO" => ETAL_DATA_TYPE_TEXTINFO,
	"FM RDS" => ETAL_DATA_TYPE_FM_RDS,              "FM RDS RAW" => ETAL_DATA_TYPE_FM_RDS_RAW
);

@etal_audio_src_list = ();
foreach $i (sort(keys %etal_audio_src_cnv)) 
{
	push @etal_audio_src_list, "$i";
}

@EtalServiceSelectMode = ("MODE_SERVICE", "MODE_DAB_SC", "MODE_DAB_SUBCH");
%EtalServiceSelectMode_cnv = (
	"MODE_SERVICE"   => ETAL_SERVSEL_MODE_SERVICE,
	"MODE_DAB_SC"    => ETAL_SERVSEL_MODE_DAB_SC,
	"MODE_DAB_SUBCH" => ETAL_SERVSEL_MODE_DAB_SUBCH
);

@EtalServiceSelectSubFunction = ("REMOVE", "APPEND", "SET");
%EtalServiceSelectSubFunction_cnv = (
	"REMOVE"    => ETAL_SERVSEL_SUBF_REMOVE,
	"APPEND"    => ETAL_SERVSEL_SUBF_APPEND,
	"SET"       => ETAL_SERVSEL_SUBF_SET
);

@EtalFrequencyBand = ("UNDEF", "FM", "FMEU", "FMUS", "HD", "FMJP", "FMEEU", "WB", "USERFM", "DRMP",
	"DRM30", "DAB3", "DABL", "AM", "LW", "MWEU", "MWUS", "SW", "CUSTAM", "USERAM");
%EtalFrequencyBand_cnv = (
	"UNDEF"     => ETAL_BAND_UNDEF,
	"FM"        => ETAL_BAND_FM,
	"FMEU"      => ETAL_BAND_FMEU,
	"FMUS"      => ETAL_BAND_FMUS,
	"HD"        => ETAL_BAND_HD,
	"FMJP"      => ETAL_BAND_FMJP,
	"FMEEU"     => ETAL_BAND_FMEEU,
	"WB"        => ETAL_BAND_WB,
	"USERFM"    => ETAL_BAND_USERFM,
	"DRMP"      => ETAL_BAND_DRMP,
	"DRM30"     => ETAL_BAND_DRM30,
	"DAB3"      => ETAL_BAND_DAB3,
	"DABL"      => ETAL_BAND_DABL,
	"AM"        => ETAL_BAND_AM,
	"LW"        => ETAL_BAND_LW,
	"MWEU"      => ETAL_BAND_MWEU,
	"MWUS"      => ETAL_BAND_MWUS,
	"SW"        => ETAL_BAND_SW,
	"CUSTAM"    => ETAL_BAND_CUSTAM,
	"USERAM"    => ETAL_BAND_USERAM
);

@dab_ch_list = (
	"B3 EU  5A 174928", "B3 EU  5B 176640", "B3 EU  5C 178352", "B3 EU  5D 180064",
	"B3 EU  6A 181936", "B3 EU  6B 183648", "B3 EU  6C 185360", "B3 EU  6D 187072",
	"B3 EU  7A 188928", "B3 EU  7B 190640", "B3 EU  7C 192352", "B3 EU  7D 194064",
	"B3 EU  8A 195936", "B3 EU  8B 197648", "B3 EU  8C 199360", "B3 EU  8D 201072",
	"B3 EU  9A 202928", "B3 EU  9B 204640", "B3 EU  9C 206352", "B3 EU  9D 208064",
	"B3 EU 10A 209936", "B3 EU 10N 210096", "B3 EU 10B 211648", "B3 EU 10C 213360",
	"B3 EU 10D 215072", "B3 EU 11A 216928", "B3 EU 11N 217088", "B3 EU 11B 218640",
	"B3 EU 11C 220352", "B3 EU 11D 222064", "B3 EU 12A 223936", "B3 EU 12N 224096",
	"B3 EU 12B 225648", "B3 EU 12C 227360", "B3 EU 12D 229072", "B3 EU 13A 230784",
	"B3 EU 13B 232496", "B3 EU 13C 234208", "B3 EU 13D 235776", "B3 EU 13E 237488",
	"B3 EU 13F 239200",

	"B3 CN  6A 168160", "B3 CN  6B 169872", "B3 CN  6C 171584", "B3 CN  6D 173296",
	"B3 CN  6N 175008", "B3 CN  7A 176720", "B3 CN  7B 178432", "B3 CN  7C 180144",
	"B3 CN  7D 181856", "B3 CN  8A 184160", "B3 CN  8B 185872", "B3 CN  8C 187584",
	"B3 CN  8D 189296", "B3 CN  8N 191008", "B3 CN  9A 192720", "B3 CN  9B 194432",
	"B3 CN  9C 196144", "B3 CN  9D 197856",
	"B3 CN 10A 200160", "B3 CN 10B 201872", "B3 CN 10C 203584", "B3 CN 10D 205296",
	"B3 CN 10N 207008", "B3 CN 11A 208720", "B3 CN 11B 210432", "B3 CN 11C 212144",
	"B3 CN 11D 213856", "B3 CN 12A 216432", "B3 CN 12B 218144", "B3 CN 12C 219856",

	"B3 KR  7A 175280", "B3 KR  7B 177008", "B3 KR  7C 178736", "B3 KR  8A 181280",
	"B3 KR  8B 183008", "B3 KR  8C 184736", "B3 KR  9A 187280", "B3 KR  9B 189008",
	"B3 KR  9C 190736",
	"B3 KR 10A 193280", "B3 KR 10B 195008", "B3 KR 10C 196736", "B3 KR 11A 199280",
	"B3 KR 11B 201008", "B3 KR 11C 202736", "B3 KR 12A 205280", "B3 KR 12B 207008",
	"B3 KR 12C 208736", "B3 KR 13A 211280", "B3 KR 13B 213008", "B3 KR 13C 214736",

	"BL EU  LA 1452960", "BL EU  LB 1454672", "BL EU  LC 1456384", "BL EU  LD 1458096",
	"BL EU  LE 1459808", "BL EU  LF 1461520", "BL EU  LG 1463232", "BL EU  LH 1464944",
	"BL EU  LI 1466656", "BL EU  LJ 1468368", "BL EU  LK 1470080", "BL EU  LL 1471792",
	"BL EU  LM 1473504", "BL EU  LN 1475216", "BL EU  LO 1476928", "BL EU  LP 1478640",
	"BL EU  LQ 1480352", "BL EU  LR 1482064", "BL EU  LS 1483776", "BL EU  LT 1485488",
	"BL EU  LU 1487200", "BL EU  LV 1488912",

	"BL CA  L1 1452816", "BL CA  L2 1454560", "BL CA  L3 1456304", "BL CA  L4 1458048",
	"BL CA  L5 1459792", "BL CA  L6 1461536", "BL CA  L7 1463280", "BL CA  L8 1465024",
	"BL CA  L9 1466768",
	"BL CA L10 1468512", "BL CA L11 1470256", "BL CA L12 1472000", "BL CA L13 1473744",
	"BL CA L14 1475488", "BL CA L15 1477232", "BL CA L16 1478976", "BL CA L17 1480720",
	"BL CA L18 1482464", "BL CA L19 1484208", "BL CA L20 1485952", "BL CA L21 1487696",
	"BL CA L22 1489440", "BL CA L23 1491184"
);

@etalSeekDirectionTy = ("up", "down");
%etalSeekDirectionTy_cnv = ( "up" => cmdDirectionUp, "down" => cmdDirectionDown);

@etalAFModeTy = ("cmdNormalMeasurement", "cmdRestartAFMeasurement");
%etalAFModeTy_cnv = (
	"cmdNormalMeasurement"      => cmdNormalMeasurement,
	"cmdRestartAFMeasurement"   => cmdRestartAFMeasurement
);

@antennaSelectionTy = ("AUTO", "FM1", "FM2");
%antennaSelectionTy_cnv = (
	"AUTO"  => 0,
	"FM1"   => 1,
	"FM2"   => 2
);

@EtalBcastQaIndicators = ("Undef", "DabFicErrorRatio", "DabFieldStrength", "DabMscBer", "FmFieldStrength", "FmFrequencyOffset",
	"FmModulationDetector", "FmMultipath", "FmUltrasonicNoise", "HdQI", "HdCdToNo", "HdDSQM");
%EtalBcastQaIndicators_cnv = (
	"Undef"  => 0, "DabFicErrorRatio" => 1, "DabFieldStrength" => 2, "DabMscBer" => 3, "FmFieldStrength" => 4, "FmFrequencyOffset" => 5, 
	"FmModulationDetector" => 6, "FmMultipath" => 7, "FmUltrasonicNoise" => 8, "HdQI" => 9, "HdCdToNo" => 10, "HdDSQM" => 11
);

for($i = 0; $i < ETAL_MAX_QUALITY_PER_MONITOR; $i++)
{
	$monitoredIndicators[$i] = [$EtalBcastQaIndicators[0], 0, 0, 0];
}

@ServiceBitmapList = ("NONE", "EPG_RAW", "SLS", "SLS_XPAD", "TPEG_RAW", "TPEG_SNI", "SLI", "EPG_BIN", "EPG_SRV", "EPG_PRG", "EPG_LOGO",
	"JML_OBJ", "FIDC", "TMC", "DLPLUS", "PSD");
%ServiceBitmap_cnv = ( "NONE" => 0x00000000, "EPG_RAW" => 0x00000001, "SLS" => 0x00000002, "SLS_XPAD" => 0x00000004, "TPEG_RAW" => 0x00000008,
	"TPEG_SNI" => 0x00000010, "SLI" => 0x00000020, "EPG_BIN" => 0x00000040, "EPG_SRV" => 0x00000080, "EPG_PRG" => 0x00000100,
	"EPG_LOGO" => 0x00000200, "JML_OBJ" => 0x00000400, "FIDC" => 0x00000800, "TMC" => 0x00001000, "DLPLUS" => 0x00002000, "PSD" => 0x00004000);

@EtalDataService_EpgLogoType = ("UNKNOWN", "UNRESTRICTED", "MONO_SQUARE", "COLOUR_SQUARE",
	"MONO_RECTANGLE", "COLOUR_RECTANGLE");
%EtalDataService_EpgLogoType_cnv = ("UNKNOWN" => 0, "UNRESTRICTED" => 1, "MONO_SQUARE" => 2, 
	"COLOUR_SQUARE" => 3, "MONO_RECTANGLE" => 4, "COLOUR_RECTANGLE" => 5);

@etalChannelTy = ("undef", "foreground", "background", "both");
%etalChannelTy_cnv = ("undef" => ETAL_CHN_UNDEF, "foreground" => ETAL_CHN_FOREGROUND, 
	"background" => ETAL_CHN_BACKGROUND, "both" => ETAL_CHN_BOTH);

@EtalPathName = ("UNDEF", "DAB_1", "DAB_2", "FM_FG", "FM_BG", "AM", "FM_HD_FG", "FM_HD_BG", "AM_HD", "DRM_1", "DRM_2");
%EtalPathName_cnv = ("UNDEF" => ETAL_PATH_NAME_UNDEF, "DAB_1" => ETAL_PATH_NAME_DAB_1, "DAB_2" => ETAL_PATH_NAME_DAB_2, 
	"FM_FG" => ETAL_PATH_NAME_FM_FG, "FM_BG" => ETAL_PATH_NAME_FM_BG, "AM" => ETAL_PATH_NAME_AM, "FM_HD_FG" => ETAL_PATH_NAME_FM_HD_FG, 
	"FM_HD_BG" => ETAL_PATH_NAME_FM_HD_BG, "AM_HD" => ETAL_PATH_NAME_AM_HD, "DRM_1" => ETAL_PATH_NAME_DRM_1, "DRM_2" => ETAL_PATH_NAME_DRM_2);

	$etaldemo_started_flag = 0;

# local functions

sub gui_print {
	# lock $guiPrintLock to protect function gui_print from simultaneous thread calls
	lock($guiPrintLock);
	$str = shift;

	# convert line end to \r because of C fgets linux that doesn't like the \r\n of DOS.
	if ((!$opt_n) && ($str =~ /\n$/)) {
		chomp($str);
		$str =~ s/$/\r/g;
	}

	print "$str";
}

sub newUserCmdSend {
	if (($#UserCmdList == -1) || ($UserCmdIdx == -1) || ($UserCmdIdx > $#UserCmdList) ||
		(($UserCmdIdx > -1) && ($UserCmdIdx <= $#UserCmdList) && ($usercmd ne $UserCmdList[$UserCmdIdx])))
	{
		push @UserCmdList, $usercmd;
		$UserCmdIdx = $#UserCmdList + 1;
	}
	gui_print("$usercmd\n");
	$usercmd = "";
}

sub gui_add_processing_feature {
	$frame = shift;

	$frame->Label(-text => "Proc Feat:")->pack(-side => "left");
	$pftfmvpa = $pftantdiv = $pfthdrdrmdigit = $pfthdrblend = $pftall = 0;
	$pftunspec = 1;
	$processingFeatures = ETAL_PROCESSING_FEATURE_UNSPECIFIED;
	$frame->Checkbutton(-text => "FM vpa", -variable => \$pftfmvpa, -command => sub { 
		if (($pftall) || ($pftunspec)) { $processingFeatures = ETAL_PROCESSING_FEATURE_NONE; }
		$pftall = $pftunspec = 0;
		if ($pftfmvpa) { $processingFeatures |= ETAL_PROCESSING_FEATURE_FM_VPA; }
		else { $processingFeatures &= ~ETAL_PROCESSING_FEATURE_FM_VPA; }
	})->pack(-side => "left");
	$frame->Checkbutton(-text => "ant div", -variable => \$pftantdiv, -command => sub {
		if (($pftall) || ($pftunspec)) { $processingFeatures = ETAL_PROCESSING_FEATURE_NONE; }
		$pftall = $pftunspec = 0;
		if ($pftantdiv) { $processingFeatures |= ETAL_PROCESSING_FEATURE_ANTENNA_DIVERSITY; }
		else { $processingFeatures &= ~ETAL_PROCESSING_FEATURE_ANTENNA_DIVERSITY; }
	})->pack(-side => "left");
	$frame->Checkbutton(-text => "hdr drm digit", -variable => \$pfthdrdrmdigit, -command => sub {
		if (($pftall) || ($pftunspec)) { $processingFeatures = ETAL_PROCESSING_FEATURE_NONE; }
		$pftall = $pftunspec = 0;
		if ($pfthdrdrmdigit) { $processingFeatures |= ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF; }
		else { $processingFeatures &= ~ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF; }
	})->pack(-side => "left");
	$frame->Checkbutton(-text => "hdr blend", -variable => \$pfthdrblend, -command => sub {
		if (($pftall) || ($pftunspec)) { $processingFeatures = ETAL_PROCESSING_FEATURE_NONE; }
		$pftall = $pftunspec = 0;
		if ($pfthdrblend) { $processingFeatures |= ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING; }
		else { $processingFeatures &= ~ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING; }
	})->pack(-side => "left");
	$frame->Checkbutton(-text => "all", -variable => \$pftall, -command => sub {
		if ($pftall) { 
			$pftfmvpa = $pftantdiv = $pfthdrdrmdigit = $pfthdrblend = $pftunspec = 0;
			$processingFeatures = ETAL_PROCESSING_FEATURE_ALL;
		}
		else {
			$pftfmvpa = $pftantdiv = $pfthdrdrmdigit = $pfthdrblend = $pftall = 0;
			$processingFeatures = ETAL_PROCESSING_FEATURE_NONE;
		}
	})->pack(-side => "left");
	$frame->Checkbutton(-text => "unspec", -variable => \$pftunspec, -command => sub {
		if  ($pftunspec) { 
			$pftfmvpa = $pftantdiv = $pfthdrdrmdigit = $pfthdrblend = $pftall = 0;
			$processingFeatures = ETAL_PROCESSING_FEATURE_UNSPECIFIED;
		}
		else { 
			$pftfmvpa = $pftantdiv = $pfthdrdrmdigit = $pfthdrblend = $pftall = 0;
			$processingFeatures = ETAL_PROCESSING_FEATURE_NONE;
		}
	})->pack(-side => "left");
}

sub worker_scp_exec
{
	my ($filename) = @_;
	my ($stepCnt, $stepPos, @scpCmdList);

	#print "debug worker_scp_exec start\n";
	@scpCmdList = ();
	if (open(FHSCP, "<", "$filename"))
	{
		$stepCnt = 0;
		while (<FHSCP>)
		{
			if ((/^\s*scr_wait:\s+(\d+)/i) || (/^\s*scr_prompt: \s*(.*)/) || (/^\s*(e.*)/i))
			{
				$scpCmdList[$stepCnt] = "$_";
				$stepCnt++;
			}
		}
		close(FHSCP);
		$stepPos = 0;
		$scpExecProgress = ($stepPos * 100 / $stepCnt);
		while (($stepPos < $stepCnt) && ($scpExecAbortReq == 0))
		{
			if ($scpCmdList[$stepPos] =~ /^\s*#/)
			{
				# scp comment
			}
			elsif ($scpCmdList[$stepPos] =~ /^\s*scr_wait:\s+(\d+)/i)
			{
				select(undef, undef, undef, ($1 / 1000));
				$stepPos++;
			}
			elsif ($scpCmdList[$stepPos] =~ /^\s*scr_prompt: \s*(.*)/)
			{
				$mw->Dialog(-title => 'scr_prompt', -text => "$1", -bitmap => 'info', -default_button => 'ok', -buttons => ['ok', 'cancel'])->Show;
				$stepPos++;
			}
			elsif ($scpCmdList[$stepPos] =~ /^\s*(e.*)$/i)
			{
				gui_print("$1\n");
				$stepPos++;
			}
			$scpExecProgress = ($stepPos * 100 / $stepCnt);
		}
	}
	else
	{
		$mw->Dialog(-title => 'Error', -text => "Error to open file $filename", -bitmap => 'error', -default_button => 'ok', -buttons => ['ok'])->Show;
	}
	$scpExecAbortRsp = 1;
	#print "debug worker_scp_exec end\n";
}

sub scpUpdateProgress
{
	$scpProgress = $scpExecProgress;
	if ($scpExecAbortRsp)
	{
		$scpUpdateTimer->cancel;
		$workerScpExecThread = undef;
		$BtnStartScp->configure(-state => 'normal');
		#$mw->Dialog(-title => 'Info', -text => "scpProgress: $scpProgress", -bitmap => 'info', -default_button => 'ok', -buttons => ['ok'])->Show;
	}
}


# main

# read command line parameters
getopts("hn");
if ($opt_h) {
    print "perl guiApp.pl [-options]\n";
    print "  -h                show this help\n";
    print "  -n                Host CRLF \\n instead of \\r\n";
    exit(0); 
}

# construct main window
$mw = MainWindow->new;

# Frame UserCmd
$frameUserCmd = $mw->Frame;

# Entry Cmd
$UserCmdIdx = -1;
$usercmd = "";
$entryUserCmd = $frameUserCmd->Entry(-textvariable => \$usercmd)->pack(-side=>'left');
$entryUserCmd->bind('<Return>' => \&newUserCmdSend);
$entryUserCmd->bind('<KP_Enter>' => \&newUserCmdSend);
$entryUserCmd->bind('<Up>' => sub { 
	if (($#UserCmdList > -1) && ($UserCmdIdx > -1))
	{
		if (($UserCmdIdx > $#UserCmdList) ||
			(($UserCmdIdx > 0) && ($UserCmdIdx <= $#UserCmdList) && ($usercmd eq $UserCmdList[$UserCmdIdx])))
		{
			$UserCmdIdx--;
		}
		if (($UserCmdIdx > -1) && ($UserCmdIdx <= $#UserCmdList))
		{
			$usercmd = $UserCmdList[$UserCmdIdx];
		}
	}
});
$entryUserCmd->bind('<Down>' => sub { 
	if ($UserCmdIdx <= $#UserCmdList)
	{
		$UserCmdIdx++;
		if ($UserCmdIdx <= $#UserCmdList)
		{
			$usercmd = $UserCmdList[$UserCmdIdx];
		}
		else
		{
			$usercmd = "";
		}
	}
});

# Button Send Cmd
$frameUserCmd->Button(-text    => 'Send Cmd', -command => \&newUserCmdSend)->pack(-side=>'left');
$frameUserCmd->Button(-text    => 'CTRL+c', -command => sub { gui_print("\cc"); })->pack(-side=>'left');
$frameUserCmd->Button(-text    => 'CTRL+z', -command => sub { gui_print("\cz"); })->pack(-side=>'left');

$frameUserCmd->pack;

# Frame Start & Quit
$frameStartQuit = $mw->Frame;

# Button Quit
$frameStartQuit->Button(
	-text    => 'Quit',
	-command => sub { if ($etaldemo_started_flag == 1) { gui_print("q\n"); } if (defined($workerScpExecThread)) {$workerScpExecThread->join;} $mw->destroy; },
)->pack(-side=>'left');

# Button Start etaldemo_SS
$frameStartQuit->Button(
	-text    => 'Start etaldemo_SS',
	-command => sub { gui_print("etaldemo_SS d t 225648 f t 107700\n"); $etaldemo_started_flag = 1; },
)->pack(-side=>'left');

$frameStartQuit->Label(-text => "Cur hRecv Idx:")->pack(-side => "left");
$cur_hRecv_idx = 0;
$frameStartQuit->Optionmenu(-variable => \$cur_recv_idx, -options => [0 .. 4])->pack(-side => "left");

$frameStartQuit->pack;


# notebook etal
$frnbe = $mw->Frame();
$nbe = $frnbe->NoteBook();
$nbpe[TAB_INIT_IDX] = $nbe->add("Init", -label=>"Init");
$nbpe[TAB_CFG_IDX] = $nbe->add("Cfg", -label=>"Cfg");
$nbpe[TAB_DPATH_IDX] = $nbe->add("Dpath", -label=>"Dpath");
$nbpe[TAB_AUDIO_IDX] = $nbe->add("Audio", -label=>"Audio");
$nbpe[TAB_TUNE_IDX] = $nbe->add("Tune", -label=>"Tune");
$nbpe[TAB_SVC_IDX] = $nbe->add("SVC", -label=>"SVC");
$nbpe[TAB_DATA_IDX] = $nbe->add("Data", -label=>"Data");
$nbpe[TAB_QUAL_IDX] = $nbe->add("Qual", -label=>"Qual");
$nbpe[TAB_SEEK_IDX] = $nbe->add("Seek", -label=>"Seek");
$nbpe[TAB_AF_IDX] = $nbe->add("AF", -label=>"AF");
$nbpe[TAB_RDS_IDX] = $nbe->add("RDS", -label=>"RDS");
$nbpe[TAB_SS_IDX] = $nbe->add("SS", -label=>"SS");
$nbpe[TAB_SF_IDX] = $nbe->add("SF", -label=>"SF");
$nbpe[TAB_RDWR_IDX] = $nbe->add("RdWr", -label=>"RdWr");
$nbpe[TAB_DBG_IDX] = $nbe->add("Dbg", -label=>"Dbg");
$nbpe[TAB_SCP_IDX] = $nbe->add("Scp", -label=>"Scp");
$nbe->raise("SS");
$nbe->pack;
$frnbe->pack;

# notebook page Init
####################
# frame etal_config_receiver
$frei = $nbpe[TAB_INIT_IDX]->Frame->pack;
$frei->Button(-text => 'etal_initialize', -command => sub { gui_print("ei $EtalCountryVariant_cnv{$CountryVariantStr} ".(($dcop_enable == 0)?1:0)." ".(($tuner1_enable == 0)?1:0)." ".(($tuner2_enable == 0)?1:0)."\n"); })->pack(-side => "left");
$frei->Label(-text => "Country:")->pack(-side => "left");
$CountryVariantStr = $EtalCountryVariant[0];
$frei->Optionmenu(-variable => \$CountryVariantStr, -options => \@EtalCountryVariant)->pack(-side => "left");
$dcop_enable = 1;
$frei->Checkbutton(-text => 'DCOP', -variable => \$dcop_enable)->pack(-side => "left");
$tuner1_enable = 1;
$frei->Checkbutton(-text => 'Tuner1', -variable => \$tuner1_enable)->pack(-side => "left");
$tuner2_enable = 1;
$frei->Checkbutton(-text => 'Tuner2', -variable => \$tuner2_enable)->pack(-side => "left");
$frei->Button(-text => 'etal_reinitialize', -command => sub { gui_print("er\n"); })->pack(-side => "left");
$frei->Button(-text => 'etal_deinitialize', -command => sub { gui_print("ed\n"); })->pack(-side => "left");
$fregc = $nbpe[TAB_INIT_IDX]->Frame->pack;
$fregc->Button(-text => 'etal_reinitialize', -command => sub { gui_print("er $load_DAB_landscape $load_AMFM_landscape $load_HD_landscape\n"); })->pack(-side => "left");
$fregc->Label(-text => "load landscape:")->pack(-side => "left");
$load_DAB_landscape = 0;
$fregc->Checkbutton(-text => 'DAB', -variable => \$load_DAB_landscape)->pack(-side => "left");
$load_AMFM_landscape = 0;
$fregc->Checkbutton(-text => 'AMFM', -variable => \$load_AMFM_landscape)->pack(-side => "left");
$load_HD_landscape = 0;
$fregc->Checkbutton(-text => 'HD', -variable => \$load_HD_landscape)->pack(-side => "left");
$fregc->Button(-text => 'etal_get_capabilities', -command => sub { gui_print("egc\n"); })->pack(-side => "left");
$fregc->Button(-text => 'etal_get_version', -command => sub { gui_print("egv\n"); })->pack(-side => "left");
$fregc->Button(-text => 'etal_get_init_status', -command => sub { gui_print("egis\n"); })->pack(-side => "left");

# notebook page Cfg
###################
# frame etal_config_receiver
$frecr = $nbpe[TAB_CFG_IDX]->Frame();
$frecr->Button(-text => 'etal_config_receiver', -command => sub { 
	gui_print("ecr $cur_recv_idx $etal_std_cnv{$stdstr}");
	for($i = 0; $i < ETAL_CAPA_MAX_FRONTEND; $i++)
	{
		if ($felist[$i])
		{
			gui_print(" ".$etal_fronend_list[$i]);
		}
		else
		{
			gui_print(" ".ETAL_INVALID_HANDLE);
		}
	}
	gui_print(" $processingFeatures\n");
})->pack(-side => "left");
$frecr->Button(-text => 'etal_destroy_receiver', -command => sub { gui_print("edr $cur_recv_idx\n"); })->pack(-side => "left");
$frecr->Button(-text => 'etal_receiver_alive', -command => sub { gui_print("era $cur_recv_idx\n"); })->pack(-side => "left");
$frecr->Button(-text => 'etal_xtal_alignment', -command => sub { gui_print("exa $cur_recv_idx\n"); })->pack(-side => "left");
$frecr->pack;

# frame etal_config_receiver parameters
$frtecrp = $nbpe[TAB_CFG_IDX]->Frame->pack;
$frtecrp->Label(-text => "Std:")->pack(-side => "left");
$stdstr = $etal_std_list[0];
$frtecrp->Optionmenu(-variable => \$stdstr, -options => \@etal_std_list)->pack(-side => "left");
for($i = 0; $i < ETAL_CAPA_MAX_FRONTEND; $i++)
{
	$felist[$i] = 0;
	$frtecrp->Checkbutton(-text => "FE$i", -variable => \$felist[$i])->pack(-side => "left");
}

# frame processing features
$frepfp = $nbpe[TAB_CFG_IDX]->Frame->pack;
gui_add_processing_feature($frepfp);
$frepfp->pack;


# notebook page Dpath
#####################
# frame datapath
$frecd = $nbpe[TAB_DPATH_IDX]->Frame();
$frecd->Button(-text => 'etal_config_datapath', -command => sub { gui_print("ecd $cur_hdpath_idx $cur_recv_idx ".($etal_dpath_cnv{"$dpathtypestr"})."\n"); })->pack(-side => "left");
$frecd->Button(-text => 'etal_destroy_datapath', -command => sub { gui_print("edd $cur_hdpath_idx\n"); })->pack(-side => "left");
$frecd->pack;
$frecdp = $nbpe[TAB_DPATH_IDX]->Frame();
$frecdp->Label(-text => "hDpath idx:")->pack(-side => "left");
$cur_hdpath_idx = 0;
$frecdp->Optionmenu(-variable => \$cur_hdpath_idx, -options => [0 .. 9])->pack(-side => "left");
$frecdp->Label(-text => "Data Type:")->pack(-side => "left");
$dpathtypestr = $etal_dpath_list[0];
$frecdp->Optionmenu(-variable => \$dpathtypestr, -options => \@etal_dpath_list)->pack(-side => "left");
$frecdp->pack;


# notebook page Audio
#####################
# frame etal_config_audio_path
$frecap = $nbpe[TAB_AUDIO_IDX]->Frame->pack;
$frecap->Button(-text => 'etal_config_audio_path', -command => sub { gui_print("ecap $tuner_idx $ecap_dac $ecap_sai_out $ecap_sai_in $ecap_sai_slave_mode\n"); })->pack(-side => "left");
$frecap->Label(-text => "Tuner Idx:")->pack(-side => "left");
$tuner_idx = 0;
$frecap->Optionmenu(-variable => \$tuner_idx, -options => [0, 1, 2])->pack(-side => "left");
$ecap_dac = $ecap_sai_out = $ecap_sai_in = $ecap_sai_slave_mode = 0;
$frecap->Checkbutton(-text => 'dac', -variable => \$ecap_dac)->pack(-side => "left");
$frecap->Checkbutton(-text => 'sai_out', -variable => \$ecap_sai_out)->pack(-side => "left");
$frecap->Checkbutton(-text => 'sai_in', -variable => \$ecap_sai_in)->pack(-side => "left");
$frecap->Checkbutton(-text => 'sai_slave_mode', -variable => \$ecap_sai_slave_mode)->pack(-side => "left");

# frame etal_audio_select
$freas = $nbpe[TAB_AUDIO_IDX]->Frame->pack;
$freas->Button(-text => 'etal_audio_select', -command => sub { gui_print("eas $cur_recv_idx ".$etal_audio_src_cnv{$easrc_select}."\n"); })->pack(-side => "left");
$easrc_select = "src_STAR_AMFM";
$freas->Optionmenu(-variable => \$easrc_select, -options => \@etal_audio_src_list)->pack(-side => "left");
$freas->Button(-text => 'etal_force_mono', -command => sub { gui_print("efm $cur_recv_idx $forceMonoFlag\n"); })->pack(-side => "left");
$forceMonoFlag = 1;
$freas->Checkbutton(-text => 'mono', -variable => \$forceMonoFlag)->pack(-side => "left");
$freas->Button(-text => 'etal_mute', -command => sub { gui_print("em $cur_recv_idx $muteFlag\n"); })->pack(-side => "left");
$muteFlag = 1;
$freas->Checkbutton(-text => 'mute', -variable => \$muteFlag)->pack(-side => "left");

# frame etal_event_FM_stereo_start
$freeFss = $nbpe[TAB_AUDIO_IDX]->Frame->pack;
$freeFss->Button(-text => 'etal_event_FM_stereo_start', -command => sub { gui_print("eeFssta $cur_recv_idx\n"); })->pack(-side => "left");
$freeFss->Button(-text => 'etal_event_FM_stereo_stop', -command => sub { gui_print("eeFssto $cur_recv_idx\n"); })->pack(-side => "left");
$freeFss->Button(-text => 'etal_debug_config_audio_alignment', -command => sub { gui_print("eedcaa $AutoAlignmentForFM $AutoAlignmentForAM\n"); })->pack(-side => "left");
$freeFss->Checkbutton(-text => 'FM', -variable => \$AutoAlignmentForFM)->pack(-side => "left");
$freeFss->Checkbutton(-text => 'AM', -variable => \$AutoAlignmentForAM)->pack(-side => "left");

# notebook page Tune
####################
# frame etal_tune_receiver
$fretr = $nbpe[TAB_TUNE_IDX]->Frame->pack;
$fretr->Label(-text => "Freq (kHz):")->pack(-side => "left");
$frequency = ETAL_INVALID_FREQUENCY;
$fretr->Entry(-textvariable => \$frequency, -width => 8)->pack(-side => "left");
$fretr->Button(-text => 'etal_tune_receiver', -command => sub { gui_print("etr $cur_recv_idx $frequency\n"); })->pack(-side => "left");
$fretr->Button(-text => 'etal_tune_receiver_async', -command => sub { gui_print("etra $cur_recv_idx $frequency\n"); })->pack(-side => "left");
$fretr->Button(-text => 'etal_get_receiver_frequency', -command => sub { gui_print("egrf $cur_recv_idx\n"); })->pack(-side => "left");
$fretrp = $nbpe[TAB_TUNE_IDX]->Frame->pack;
$fretrp->Label(-text => "DAB channel:")->pack(-side => "left");
$dab_ch_sel = $dab_ch_list[0];
$fretrp->Optionmenu(-variable => \$dab_ch_sel, -options => \@dab_ch_list, -command => sub { $frequency = $dab_ch_sel; $frequency =~ s/.*\s(\d+)$/$1/; })->pack(-side => "left");
$frecbr = $nbpe[TAB_TUNE_IDX]->Frame->pack;
$frecbr->Button(-text => 'etal_change_band_receiver', -command => sub { gui_print("ecbr $cur_recv_idx $EtalFrequencyBand_cnv{$bandstr} $fmin $fmax $processingFeatures\n"); })->pack(-side => "left");
$bandstr = $EtalFrequencyBand[0];
$frecbr->Optionmenu(-variable => \$bandstr, -options => \@EtalFrequencyBand)->pack(-side => "left");
$frecbr->Label(-text => "fmin:")->pack(-side => "left");
$fmin = 0;
$frecbr->Entry(-textvariable => \$fmin, -width => 8)->pack(-side => "left");
$frecbr->Label(-text => "fmax:")->pack(-side => "left");
$fmax = 0;
$frecbr->Entry(-textvariable => \$fmax, -width => 8)->pack(-side => "left");
$frecbrp = $nbpe[TAB_TUNE_IDX]->Frame->pack;
gui_add_processing_feature($frecbrp);
$frescsta = $nbpe[TAB_TUNE_IDX]->Frame->pack;
$frescsta->Button(-text => 'etaltml_scan_start', -command => sub { gui_print("escsta $cur_recv_idx $audioPlayTime $etalSeekDirectionTy_cnv{$scanStart_directionstr} $scanStart_step\n"); })->pack(-side => "left");
$frescsta->Label(-text => "play time:")->pack(-side => "left");
$audioPlayTime = 0;
$frescsta->Entry(-textvariable => \$audioPlayTime, -width => 4)->pack(-side => "left");
$scanStart_directionstr = $etalSeekDirectionTy[0];
$frescsta->Optionmenu(-variable => \$scanStart_directionstr, -options => \@etalSeekDirectionTy)->pack(-side => "left");
$frescsta->Label(-text => "step (kHz):")->pack(-side => "left");
$scanStart_step = 100;
$frescsta->Entry(-textvariable => \$scanStart_step, -width => 8)->pack(-side => "left");
$frescsta->Button(-text => 'etaltml_scan_stop', -command => sub { gui_print("escsto $cur_recv_idx $scanStop_terminationMode\n"); })->pack(-side => "left");
$scanStop_terminationMode = 0;
$frescsta->Checkbutton(-text => 'last freq', -variable => \$scanStop_terminationMode)->pack(-side => "left");
$frelsta = $nbpe[TAB_TUNE_IDX]->Frame->pack;
$frelsta->Button(-text => 'etaltml_learn_start', -command => sub { gui_print("elsta $cur_recv_idx $EtalFrequencyBand_cnv{$learStart_bandstr} $learStart_step $learStart_nbOfFreq $learStart_mode\n"); })->pack(-side => "left");
$learStart_bandstr = $EtalFrequencyBand[0];
$frelsta->Optionmenu(-variable => \$learStart_bandstr, -options => \@EtalFrequencyBand)->pack(-side => "left");
$frelsta->Label(-text => "step (kHz):")->pack(-side => "left");
$learStart_step = 100;
$frelsta->Entry(-textvariable => \$learStart_step, -width => 8)->pack(-side => "left");
$frelsta->Label(-text => "nbOfFreq:")->pack(-side => "left");
$learStart_nbOfFreq = 1;
$frelsta->Entry(-textvariable => \$learStart_nbOfFreq, -width => 3)->pack(-side => "left");
$learStart_mode = 0;
$frelsta->Checkbutton(-text => 'sort', -variable => \$learStart_mode)->pack(-side => "left");
$frelsta->Button(-text => 'etaltml_learn_stop', -command => sub { gui_print("elsto $cur_recv_idx $learnStop_terminationMode\n"); })->pack(-side => "left");
$learnStop_terminationMode = 0;
$frelsta->Checkbutton(-text => 'last freq', -variable => \$learnStop_terminationMode)->pack(-side => "left");

# notebook page SVC
###################
# frame etal_get_current_ensemble etal_get_service_list
$fregce = $nbpe[TAB_SVC_IDX]->Frame->pack;
$fregce->Button(-text => 'etal_get_current_ensemble', -command => sub { gui_print("egce $cur_recv_idx\n"); })->pack(-side => "left");
$fregce->Button(-text => 'etal_get_ensemble_list', -command => sub { gui_print("egel\n"); })->pack(-side => "left");
$fregce->Button(-text => 'etal_get_ensemble_data', -command => sub { gui_print("eged $ueid\n"); })->pack(-side => "left");
$fregce->Button(-text => 'etal_get_service_list', -command => sub { gui_print("egsl $cur_recv_idx $ueid $bGetAudioServices $bGetDataServices\n"); })->pack(-side => "left");
$bGetAudioServices = 0;
$fregce->Checkbutton(-text => 'audioSvc', -variable => \$bGetAudioServices)->pack(-side => "left");
$bGetDataServices = 0;
$fregce->Checkbutton(-text => 'dataSvc', -variable => \$bGetDataServices)->pack(-side => "left");
$fressa = $nbpe[TAB_SVC_IDX]->Frame->pack;
$fressa->Button(-text => 'etal_get_specific_service_data_DAB', -command => sub { gui_print("egssdD $ueid $service\n"); })->pack(-side => "left");
$fressa->Button(-text => 'etal_service_select_audio', -command => sub { gui_print("essa $cur_recv_idx $EtalServiceSelectMode_cnv{$mode} $ueid $service $sc $subch\n"); })->pack(-side => "left");
$mode = $EtalServiceSelectMode[0];
$fressa->Optionmenu(-variable => \$mode, -options => \@EtalServiceSelectMode)->pack(-side => "left");
$fressa->Label(-text => "UEId:")->pack(-side => "left");
$ueid = ETAL_INVALID_UEID;
$fressa->Entry(-textvariable => \$ueid, -width => 10)->pack(-side => "left");
$fressap = $nbpe[TAB_SVC_IDX]->Frame->pack;
$fressap->Label(-text => "SID:")->pack(-side => "left");
$service = ETAL_INVALID_SID;
$fressap->Entry(-textvariable => \$service, -width => 10)->pack(-side => "left");
$fressap->Label(-text => "sc:")->pack(-side => "left");
$sc = ETAL_INVALID;
$fressap->Entry(-textvariable => \$sc, -width => 10)->pack(-side => "left");
$fressap->Label(-text => "subch:")->pack(-side => "left");
$subch = ETAL_INVALID;
$fressap->Entry(-textvariable => \$subch, -width => 10)->pack(-side => "left");
$fressd = $nbpe[TAB_SVC_IDX]->Frame->pack;
$fressd->Button(-text => 'etal_service_select_data', -command => sub { gui_print("essd $cur_hdpath_idx $EtalServiceSelectMode_cnv{$mode} $EtalServiceSelectSubFunction_cnv{$type} $ueid $service $sc $subch\n"); })->pack(-side => "left");
$fressd->Label(-text => "hDpath idx:")->pack(-side => "left");
$cur_hdpath_idx = 0;
$fressd->Optionmenu(-variable => \$cur_hdpath_idx, -options => [0 .. 9])->pack(-side => "left");
$fressd->Label(-text => "sub func type:")->pack(-side => "left");
$type = $EtalServiceSelectSubFunction[2];
$fressd->Optionmenu(-variable => \$type, -options => \@EtalServiceSelectSubFunction)->pack(-side => "left");
$fressd->Button(-text => 'etal_get_fic', -command => sub { gui_print("egf $cur_recv_idx\n"); })->pack(-side => "left");

# notebook page DATA
####################
# frame enable data service
$freeds = $nbpe[TAB_DATA_IDX]->Frame->pack;
$freeds1 = $freeds->Frame->pack(-side => 'left');
$freeds1->Button(-text => 'etal_enable_data_service', -command => sub {
	@ServiceBitmapSelected = $freedslb->curselection;
	$ServiceBitmap = ETAL_DATASERV_TYPE_NONE;
	for($i = 0; $i <= $#ServiceBitmapSelected; $i++)
	{
		$ServiceBitmap |= $ServiceBitmap_cnv{"$ServiceBitmapList[$ServiceBitmapSelected[$i]]"};
	}
	gui_print("eeds $cur_recv_idx $ServiceBitmap $ecc $eid $sid $EtalDataService_EpgLogoType_cnv{$logoType} $JMLObjectId\n"); 
})->pack;
$freeds1->Button(-text => 'etal_disable_data_service', -command => sub {
	@ServiceBitmapSelected = $freedslb->curselection;
	$ServiceBitmap = ETAL_DATASERV_TYPE_NONE;
	for($i = 0; $i <= $#ServiceBitmapSelected; $i++)
	{
		$ServiceBitmap |= $ServiceBitmap_cnv{"$ServiceBitmapList[$ServiceBitmapSelected[$i]]"};
	}
	gui_print("edds $cur_recv_idx $ServiceBitmap\n"); 
})->pack;
$freeds2 = $freeds->Frame->pack(-side => 'left');
$freedssb = $freeds2->Scrollbar()->pack(-side => "right", -fill => 'y');
$freedslb = $freeds2->Listbox(-height => 3, -selectmode => 'multiple', -listvariable => \@ServiceBitmapList, -yscrollcommand => ['set' => $freedssb])->pack(-side => "left");
$freedssb->configure(-command => ['yview' => $freedslb]);
$freeds3 = $freeds->Frame->pack(-side => 'left');
$freeds31 = $freeds3->Frame->pack;
$freeds31->Label(-text => "ecc:")->pack(-side => "left");
$ecc = 0;
$freeds31->Entry(-textvariable => \$ecc, -width => 2)->pack(-side => "left");
$freeds31->Label(-text => "eid:")->pack(-side => "left");
$eid = ETAL_INVALID_UEID;
$freeds31->Entry(-textvariable => \$eid, -width => 5)->pack(-side => "left");
$freeds31->Label(-text => "sid:")->pack(-side => "left");
$sid = ETAL_INVALID_SID;
$freeds31->Entry(-textvariable => \$sid, -width => 5)->pack(-side => "left");
$freeds31->Label(-text => "JMLObjectId:")->pack(-side => "left");
$JMLObjectId = 0;
$freeds31->Entry(-textvariable => \$JMLObjectId, -width => 5)->pack(-side => "left");
$freeds32 = $freeds3->Frame->pack;
$freeds32->Label(-text => "logo:")->pack(-side => "left");
$freeds32->Optionmenu(-variable => \$logoType, -options => \@EtalDataService_EpgLogoType)->pack(-side => "left");
$fresP = $nbpe[TAB_DATA_IDX]->Frame->pack;
$fresP->Button(-text => 'etal_setup_PSD', -command => sub { 
	if ($pConfigLenSet)
	{
		gui_print("esP $cur_recv_idx $PSDTitleLength $PSDArtistLength $PSDAlbumLength $PSDGenreLength $PSDCommentShortLength $PSDCommentLength $PSDUFIDLength $PSDCommercialPriceLength $PSDCommercialContactLength $PSDCommercialSellerLength $PSDCommercialDescriptionLength $PSDXHDRLength\n");
	}
	else
	{
		gui_print("esP $cur_recv_idx\n");
	}
})->pack(-side => "left");
$pConfigLenSet = 0;
$fresP->Checkbutton(-text => 'set', -variable => \$pConfigLenSet)->pack(-side => "left");
$PSDTitleLength = $PSDArtistLength = $PSDAlbumLength = $PSDGenreLength = $PSDCommentShortLength = $PSDCommentLength = $PSDUFIDLength = 0;
$PSDCommercialPriceLength = $PSDCommercialContactLength = $PSDCommercialSellerLength = $PSDCommercialDescriptionLength = $PSDXHDRLength = 0;
$fresP->Label(-text => "Title:")->pack(-side => "left");
$fresP->Entry(-textvariable => \$PSDTitleLength, -width => 3)->pack(-side => "left");
$fresP->Label(-text => "Artist:")->pack(-side => "left");
$fresP->Entry(-textvariable => \$PSDArtistLength, -width => 3)->pack(-side => "left");
$fresP->Label(-text => "Album:")->pack(-side => "left");
$fresP->Entry(-textvariable => \$PSDAlbumLength, -width => 3)->pack(-side => "left");
$fresP->Label(-text => "Genre:")->pack(-side => "left");
$fresP->Entry(-textvariable => \$PSDGenreLength, -width => 3)->pack(-side => "left");
$fresP->Label(-text => "CommentShort:")->pack(-side => "left");
$fresP->Entry(-textvariable => \$PSDCommentShortLength, -width => 3)->pack(-side => "left");
$fresP->Label(-text => "Comment:")->pack(-side => "left");
$fresP->Entry(-textvariable => \$PSDCommentLength, -width => 3)->pack(-side => "left");
$fresP->Label(-text => "UFID:")->pack(-side => "left");
$fresP->Entry(-textvariable => \$PSDUFIDLength, -width => 3)->pack(-side => "left");
$fresP2 = $nbpe[TAB_DATA_IDX]->Frame->pack;
$fresP2->Label(-text => "CommercialPrice:")->pack(-side => "left");
$fresP2->Entry(-textvariable => \$PSDCommercialPriceLength, -width => 3)->pack(-side => "left");
$fresP2->Label(-text => "CommercialContact:")->pack(-side => "left");
$fresP2->Entry(-textvariable => \$PSDCommercialContactLength, -width => 3)->pack(-side => "left");
$fresP2->Label(-text => "CommercialSeller:")->pack(-side => "left");
$fresP2->Entry(-textvariable => \$PSDCommercialSellerLength, -width => 3)->pack(-side => "left");
$fresP2->Label(-text => "CommercialDescription:")->pack(-side => "left");
$fresP2->Entry(-textvariable => \$PSDCommercialDescriptionLength, -width => 3)->pack(-side => "left");
$fresP2->Label(-text => "XHDR:")->pack(-side => "left");
$fresP2->Entry(-textvariable => \$PSDXHDRLength, -width => 3)->pack(-side => "left");
# TODO add Listbox for etal_setup_PSD parameter PSDServiceEnableBitmap

# notebook page QUAL
####################
# Frame get reception quality
$fregrq = $nbpe[TAB_QUAL_IDX]->Frame->pack;
$fregrq->Button(-text => 'etal_get_reception_quality', -command => sub { gui_print("egrq $cur_recv_idx\n"); })->pack(-side => "left");
$fregrq->Button(-text => 'etal_get_channel_quality', -command => sub { gui_print("egcq $cur_recv_idx\n"); })->pack(-side => "left");
$frecrqm = $nbpe[TAB_QUAL_IDX]->Frame->pack;
$frecrqm->Button(-text => 'etal_config_reception_quality_monitor', -command => sub {
	gui_print("ecrqm $cur_hmonitor_idx $cur_recv_idx");
	for($i = 0; $i < ETAL_MAX_QUALITY_PER_MONITOR; $i++)
	{
		gui_print(" ".$EtalBcastQaIndicators_cnv{$monitoredIndicators[$i][0]}." ".$monitoredIndicators[$i][1]." ".$monitoredIndicators[$i][2]." ".$monitoredIndicators[$i][3]);
	}
	gui_print(" $monitoredContext\n");
})->pack(-side => "left");
$frecrqm->Label(-text => "hMonitor idx:")->pack(-side => "left");
$cur_hmonitor_idx = 0;
$frecrqm->Optionmenu(-variable => \$cur_hmonitor_idx, -options => [0 .. 9])->pack(-side => "left");
$frecrqm->Label(-text => "ctx:")->pack(-side => "left");
$monitoredContext = 0;
$frecrqm->Entry(-textvariable => \$monitoredContext, -width => 8)->pack(-side => "left");
$frecrqm->Button(-text => 'etal_destroy_reception_quality_monitor', -command => sub { gui_print("edrqm $cur_hmonitor_idx\n"); })->pack(-side => "left");
$frecrqmp = $nbpe[TAB_QUAL_IDX]->Frame->pack;
$frecrqmp->Label(-text => "idx:")->pack(-side => "left");
$monitoredIndicators_idx = 0;
$frecrqmp->Optionmenu(-variable => \$monitoredIndicators_idx, -options => [0 .. (ETAL_MAX_QUALITY_PER_MONITOR - 1)], -command => sub {
	$MonitoredIndicator = $monitoredIndicators[$monitoredIndicators_idx][0];
	$InferiorValue = $monitoredIndicators[$monitoredIndicators_idx][1];
	$SuperiorValue = $monitoredIndicators[$monitoredIndicators_idx][2];
	$UpdateFrequency = $monitoredIndicators[$monitoredIndicators_idx][3];
})->pack(-side => "left");
$MonitoredIndicator = $EtalBcastQaIndicators[0];
$frecrqmp->Optionmenu(-variable => \$MonitoredIndicator, -options => \@EtalBcastQaIndicators)->pack(-side => "left");
$frecrqmp->Label(-text => "infVal:")->pack(-side => "left");
$InferiorValue = $monitoredIndicators[$monitoredIndicators_idx][1];
$frecrqmp->Entry(-textvariable => \$InferiorValue, -width => 8)->pack(-side => "left");
$frecrqmp->Label(-text => "supVal:")->pack(-side => "left");
$SuperiorValue = $monitoredIndicators[$monitoredIndicators_idx][2];
$frecrqmp->Entry(-textvariable => \$SuperiorValue, -width => 8)->pack(-side => "left");
$frecrqmp->Label(-text => "updFrq:")->pack(-side => "left");
$UpdateFrequency = $monitoredIndicators[$monitoredIndicators_idx][3];
$frecrqmp->Entry(-textvariable => \$UpdateFrequency, -width => 8)->pack(-side => "left");
$frecrqmp->Button(-text => 'set', -command => sub {
	$monitoredIndicators[$monitoredIndicators_idx] =[$MonitoredIndicator , $InferiorValue, $SuperiorValue, $UpdateFrequency];
})->pack(-side => "left");
$frecrqmp->Button(-text => 'clear', -command => sub {
	$monitoredIndicators[$monitoredIndicators_idx] =[$EtalBcastQaIndicators[0] , 0, 0, 0];
	$MonitoredIndicator = $EtalBcastQaIndicators[0];
	$InferiorValue = $SuperiorValue = $UpdateFrequency = 0;
})->pack(-side => "left");
$fregCd = $nbpe[TAB_QUAL_IDX]->Frame->pack;
$fregCd->Button(-text => 'etal_get_CF_data', -command => sub { gui_print("egCd $cur_recv_idx $nbOfAverage $period\n"); })->pack(-side => "left");
$fregCd->Label(-text => "nbOfAverage:")->pack(-side => "left");
$nbOfAverage = 0;
$fregCd->Entry(-textvariable => \$nbOfAverage, -width => 8)->pack(-side => "left");
$fregCd->Label(-text => "period:")->pack(-side => "left");
$period = 0;
$fregCd->Entry(-textvariable => \$period, -width => 8)->pack(-side => "left");

# notebook page SEEK
####################
# Frame seek start manual
$fressstam = $nbpe[TAB_SEEK_IDX]->Frame->pack;
$fressstam->Button(-text => 'etal_seek_start_manual', -command => sub { gui_print("esstam $cur_recv_idx $etalSeekDirectionTy_cnv{$directionstr} $step\n"); })->pack(-side => "left");
$directionstr = $etalSeekDirectionTy[0];
$fressstam->Optionmenu(-variable => \$directionstr, -options => \@etalSeekDirectionTy)->pack(-side => "left");
$fressstam->Label(-text => "step/freq (kHz):")->pack(-side => "left");
$step = 100;
$fressstam->Entry(-textvariable => \$step, -width => 8)->pack(-side => "left");
$fressstam->Button(-text => 'etal_seek_continue_manual', -command => sub { gui_print("escm $cur_recv_idx\n"); })->pack(-side => "left");
$fressstom = $nbpe[TAB_SEEK_IDX]->Frame->pack;
$fressstom->Button(-text => 'etal_seek_stop_manual', -command => sub { 
	if ($exitSeekAction) {
		gui_print("esstom $cur_recv_idx ".cmdAudioMuted."\n");
	}
	else {
		gui_print("esstom $cur_recv_idx ".cmdAudioUnmuted."\n");
	}
})->pack(-side => "left");
$exitSeekAction = 0;
$fressstom->Checkbutton(-text => 'mute', -variable => \$exitSeekAction)->pack(-side => "left");
$fressstom->Button(-text => 'etal_seek_get_status_manual', -command => sub { gui_print("esgsm $cur_recv_idx\n"); })->pack(-side => "left");
$freass = $nbpe[TAB_SEEK_IDX]->Frame->pack;
$freass->Button(-text => 'etal_autoseek_start', -command => sub {
	if ($exitSeekAction) {
		gui_print("essta $cur_recv_idx $etalSeekDirectionTy_cnv{$directionstr} $step ".cmdAudioMuted." $seekHDSPS $updateStopFrequency\n");
	}
	else {
		gui_print("essta $cur_recv_idx $etalSeekDirectionTy_cnv{$directionstr} $step ".cmdAudioUnmuted." $seekHDSPS $updateStopFrequency\n");
	} 
})->pack(-side => "left");
$seekHDSPS = 0;
$freass->Checkbutton(-text => 'seekSPS', -variable => \$seekHDSPS)->pack(-side => "left");
$updateStopFrequency = 0;
$freass->Checkbutton(-text => 'updateStopFreq', -variable => \$updateStopFrequency)->pack(-side => "left");
$freass->Button(-text => 'etal_autoseek_stop', -command => sub { gui_print("essto $cur_recv_idx $terminationMode\n"); })->pack(-side => "left");
$terminationMode = 0;
$freass->Checkbutton(-text => 'last freq', -variable => \$terminationMode)->pack(-side => "left");
$fresstv = $nbpe[TAB_SEEK_IDX]->Frame->pack;
$fresstv->Button(-text => 'etal_set_autoseek_thresholds_value', -command => sub { gui_print("esstv $cur_recv_idx $rffs $bbfs $detune $adjacentch $multipath\n"); })->pack(-side => "left");
$fresstv->Label(-text => "RFFS:")->pack(-side => "left");
$fresstv->Entry(-textvariable => \$rffs, -width => 4)->pack(-side => "left");
$fresstv->Label(-text => "BBFS:")->pack(-side => "left");
$fresstv->Entry(-textvariable => \$bbfs, -width => 4)->pack(-side => "left");
$fresstv->Label(-text => "det:")->pack(-side => "left");
$fresstv->Entry(-textvariable => \$detune, -width => 4)->pack(-side => "left");
$fresstv->Label(-text => "adj:")->pack(-side => "left");
$fresstv->Entry(-textvariable => \$adjacentch, -width => 4)->pack(-side => "left");
$fresstv->Label(-text => "mp:")->pack(-side => "left");
$fresstv->Entry(-textvariable => \$multipath, -width => 4)->pack(-side => "left");

# notebook page AF
##################
# Frame AF start end
$freAs = $nbpe[TAB_AF_IDX]->Frame->pack;
$freAs->Button(-text => 'etal_AF_start', -command => sub { gui_print("eAst $cur_recv_idx $etalAFModeTy_cnv{$AFModestr} $alternateFrequency $antennaSelectionTy_cnv{$antennaSelectionstr}\n"); })->pack(-side => "left");
$AFModestr = $etalAFModeTy[1];
$freAs->Optionmenu(-variable => \$AFModestr, -options => \@etalAFModeTy)->pack(-side => "left");
$freAs->Button(-text => 'etal_AF_end', -command => sub { gui_print("eAe $cur_recv_idx $alternateFrequency\n"); })->pack(-side => "left");
$freAs->Button(-text => 'etal_AF_check', -command => sub { gui_print("eAc $cur_recv_idx $alternateFrequency $antennaSelectionTy_cnv{$antennaSelectionstr}\n"); })->pack(-side => "left");
$freAs->Button(-text => 'etal_AF_switch', -command => sub { gui_print("eAsw $cur_recv_idx $alternateFrequency\n"); })->pack(-side => "left");
$freAsp = $nbpe[TAB_AF_IDX]->Frame->pack;
$freAsp->Label(-text => "Alt Freq (kHz):")->pack(-side => "left");
$alternateFrequency = 0;
$freAsp->Entry(-textvariable => \$alternateFrequency, -width => 8)->pack(-side => "left");
$freAsp->Label(-text => "Alt DAB ch:")->pack(-side => "left");
$alt_dab_ch_sel = $dab_ch_list[0];
$freAsp->Optionmenu(-variable => \$alt_dab_ch_sel, -options => \@dab_ch_list, -command => sub { $alternateFrequency = $alt_dab_ch_sel; $alternateFrequency =~ s/.*\s(\d+)$/$1/; })->pack(-side => "left");
$freAsp->Label(-text => "ant:")->pack(-side => "left");
$antennaSelectionstr = $antennaSelectionTy[0];
$freAsp->Optionmenu(-variable => \$antennaSelectionstr, -options => \@antennaSelectionTy)->pack(-side => "left");
$freAsm = $nbpe[TAB_AF_IDX]->Frame->pack;
$freAsm->Button(-text => 'etal_AF_search_manual', -command => sub {
	$AFList = $AFListstr;
	$AFList =~ s/(\s)\s+/$1/g; 
	gui_print("eAsm $cur_recv_idx $antennaSelectionTy_cnv{$antennaSelectionstr} $AFList\n"); 
})->pack(-side => "left");
$freAsm->Label(-text => "AFList:")->pack(-side => "left");
$AFListstr = "0";
$freAsm->Entry(-textvariable => \$AFListstr)->pack(-side => "left");

# notebook page RDS
###################
# Frame start RDS
$fresR = $nbpe[TAB_RDS_IDX]->Frame->pack;
$fresR->Button(-text => 'etal_start_RDS', -command => sub { gui_print("estaR $cur_recv_idx $forceFastPi $numPi $rbdsmode $errThresh $groupOutEn\n"); })->pack(-side => "left");
$forceFastPi = 0;
$fresR->Checkbutton(-text => 'forceFastPi', -variable => \$forceFastPi)->pack(-side => "left");
$fresR->Label(-text => "numPi:")->pack(-side => "left");
$numPi = 0;
$fresR->Optionmenu(-variable => \$numPi, -options => [0 .. 16])->pack(-side => "left");
$rbdsmode = 0;
$fresR->Checkbutton(-text => 'RBDS', -variable => \$rbdsmode)->pack(-side => "left");
$fresR->Label(-text => "errThresh:")->pack(-side => "left");
$errThresh = 0;
$fresR->Optionmenu(-variable => \$errThresh, -options => [0 .. 6])->pack(-side => "left");
$groupOutEn = 0;
$fresR->Checkbutton(-text => 'groupOutEn', -variable => \$groupOutEn)->pack(-side => "left");
$fresR->Button(-text => 'etal_stop_RDS', -command => sub { gui_print("estoR $cur_recv_idx\n"); })->pack(-side => "left");
$frestat = $nbpe[TAB_RDS_IDX]->Frame->pack;
$frestat->Button(-text => 'etaltml_start_textinfo', -command => sub { gui_print("estat $cur_recv_idx\n"); })->pack(-side => "left");
$frestat->Button(-text => 'etaltml_get_textinfo', -command => sub { gui_print("egt $cur_recv_idx\n"); })->pack(-side => "left");
$frestat->Button(-text => 'etaltml_stop_textinfo', -command => sub { gui_print("estot $cur_recv_idx\n"); })->pack(-side => "left");
$frestat->Button(-text => 'etaltml_start_decoded_RDS', -command => sub { gui_print("estadR $cur_recv_idx $RDSServiceList\n"); })->pack(-side => "left");
$frestat->Button(-text => 'etaltml_get_decoded_RDS', -command => sub { gui_print("egdR $cur_recv_idx\n"); })->pack(-side => "left");
$frestadR = $nbpe[TAB_RDS_IDX]->Frame->pack;
$frestadR->Button(-text => 'etaltml_stop_decoded_RDS', -command => sub { gui_print("estodR $cur_recv_idx $RDSServiceList\n"); })->pack(-side => "left");
$frestadR->Label(-text => "RDSServiceList:")->pack(-side => "left");
$RDSServiceList_PS = $RDSServiceList_DI = $RDSServiceList_PI = $RDSServiceList_TOM = $RDSServiceList_RT = 0;
$RDSServiceList_AF = $RDSServiceList_PTY = $RDSServiceList_TP = $RDSServiceList_TA = $RDSServiceList_MS = 0;
$RDSServiceList_ALL = $RDSServiceList = 0;
$frestadR->Checkbutton(-text => "PS", -variable => \$RDSServiceList_PS, -command => sub { 
	if ($RDSServiceList_ALL) { $RDSServiceList_ALL = $RDSServiceList = 0; }
	if ($RDSServiceList_PS) { $RDSServiceList |= ETAL_DECODED_RDS_VALID_PS; }
	else { $RDSServiceList &= ~ETAL_DECODED_RDS_VALID_PS; }
})->pack(-side => "left");
$frestadR->Checkbutton(-text => "DI", -variable => \$RDSServiceList_DI, -command => sub {
	if ($RDSServiceList_ALL) { $RDSServiceList_ALL = $RDSServiceList = 0; }
	if ($RDSServiceList_DI) { $RDSServiceList |= ETAL_DECODED_RDS_VALID_DI; }
	else { $RDSServiceList &= ~ETAL_DECODED_RDS_VALID_DI; }
})->pack(-side => "left");
$frestadR->Checkbutton(-text => "PI", -variable => \$RDSServiceList_PI, -command => sub {
	if ($RDSServiceList_ALL) { $RDSServiceList_ALL = $RDSServiceList = 0; }
	if ($RDSServiceList_PI) { $RDSServiceList |= ETAL_DECODED_RDS_VALID_PI; }
	else { $RDSServiceList &= ~ETAL_DECODED_RDS_VALID_PI; }
})->pack(-side => "left");
$frestadR->Checkbutton(-text => "TOM", -variable => \$RDSServiceList_TOM, -command => sub {
	if ($RDSServiceList_ALL) { $RDSServiceList_ALL = $RDSServiceList = 0; }
	if ($RDSServiceList_TOM) { $RDSServiceList |= ETAL_DECODED_RDS_VALID_TOM; }
	else { $RDSServiceList &= ~ETAL_DECODED_RDS_VALID_TOM; }
})->pack(-side => "left");
$frestadR->Checkbutton(-text => "RT", -variable => \$RDSServiceList_RT, -command => sub {
	if ($RDSServiceList_ALL) { $RDSServiceList_ALL = $RDSServiceList = 0; }
	if ($RDSServiceList_RT) { $RDSServiceList |= ETAL_DECODED_RDS_VALID_RT; }
	else { $RDSServiceList &= ~ETAL_DECODED_RDS_VALID_RT; }
})->pack(-side => "left");
$frestadR->Checkbutton(-text => "AF", -variable => \$RDSServiceList_AF, -command => sub {
	if ($RDSServiceList_ALL) { $RDSServiceList_ALL = $RDSServiceList = 0; }
	if ($RDSServiceList_RT) { $RDSServiceList |= ETAL_DECODED_RDS_VALID_AF; }
	else { $RDSServiceList &= ~ETAL_DECODED_RDS_VALID_AF; }
})->pack(-side => "left");
$frestadR->Checkbutton(-text => "PTY", -variable => \$RDSServiceList_PTY, -command => sub {
	if ($RDSServiceList_ALL) { $RDSServiceList_ALL = $RDSServiceList = 0; }
	if ($RDSServiceList_PTY) { $RDSServiceList |= ETAL_DECODED_RDS_VALID_PTY; }
	else { $RDSServiceList &= ~ETAL_DECODED_RDS_VALID_PTY; }
})->pack(-side => "left");
$frestadR->Checkbutton(-text => "TP", -variable => \$RDSServiceList_TP, -command => sub {
	if ($RDSServiceList_ALL) { $RDSServiceList_ALL = $RDSServiceList = 0; }
	if ($RDSServiceList_TP) { $RDSServiceList |= ETAL_DECODED_RDS_VALID_TP; }
	else { $RDSServiceList &= ~ETAL_DECODED_RDS_VALID_TP; }
})->pack(-side => "left");
$frestadR->Checkbutton(-text => "TA", -variable => \$RDSServiceList_TA, -command => sub {
	if ($RDSServiceList_ALL) { $RDSServiceList_ALL = $RDSServiceList = 0; }
	if ($RDSServiceList_TA) { $RDSServiceList |= ETAL_DECODED_RDS_VALID_TA; }
	else { $RDSServiceList &= ~ETAL_DECODED_RDS_VALID_TA; }
})->pack(-side => "left");
$frestadR->Checkbutton(-text => "MS", -variable => \$RDSServiceList_MS, -command => sub {
	if ($RDSServiceList_ALL) { $RDSServiceList_ALL = $RDSServiceList = 0; }
	if ($RDSServiceList_MS) { $RDSServiceList |= ETAL_DECODED_RDS_VALID_MS; }
	else { $RDSServiceList &= ~ETAL_DECODED_RDS_VALID_MS; }
})->pack(-side => "left");
$frestadR->Checkbutton(-text => "ALL", -variable => \$RDSServiceList_ALL, -command => sub {
	$RDSServiceList_PS = $RDSServiceList_DI = $RDSServiceList_PI = $RDSServiceList_TOM = $RDSServiceList_RT = 0;
	$RDSServiceList_AF = $RDSServiceList_PTY = $RDSServiceList_TP = $RDSServiceList_TA = $RDSServiceList_MS = 0;
	if ($RDSServiceList_ALL) { $RDSServiceList = ETAL_DECODED_RDS_VALID_ALL; }
	else { $RDSServiceList = 0; }
})->pack(-side => "left");
$fregvRbm = $nbpe[TAB_RDS_IDX]->Frame->pack;
$fregvRbm->Button(-text => 'etaltml_get_validated_RDS_block_manual', -command => sub { gui_print("egvRbm $cur_recv_idx\n"); })->pack(-side => "left");
$fregvRbm->Button(-text => 'etaltml_RDS_AF', -command => sub { gui_print("eRA $cur_recv_idx $hReceiverB $AFOn\n"); })->pack(-side => "left");
$fregvRbm->Label(-text => "hReceiverB Idx:")->pack(-side => "left");
$hReceiverB = 1;
$fregvRbm->Optionmenu(-variable => \$hReceiverB, -options => [0 .. 4])->pack(-side => "left");
$AFOn = 0;
$fregvRbm->Checkbutton(-text => 'AFOn', -variable => \$AFOn)->pack(-side=>'left');
$fregvRbm->Button(-text => 'etaltml_RDS_TA', -command => sub { gui_print("eRT $cur_recv_idx $TAOn\n"); })->pack(-side => "left");
$TAOn = 0;
$fregvRbm->Checkbutton(-text => 'TAOn', -variable => \$TAOn)->pack(-side=>'left');
$freRE = $nbpe[TAB_RDS_IDX]->Frame->pack;
$freRE->Button(-text => 'etaltml_RDS_EON', -command => sub { gui_print("eRE $cur_recv_idx $EONOn\n"); })->pack(-side => "left");
$EONOn = 0;
$freRE->Checkbutton(-text => 'EONOn', -variable => \$EONOn)->pack(-side=>'left');
$freRE->Button(-text => 'etaltml_RDS_REG', -command => sub { gui_print("eRR $cur_recv_idx $REGOn\n"); })->pack(-side => "left");
$REGOn = 0;
$freRE->Checkbutton(-text => 'REGOn', -variable => \$REGOn)->pack(-side=>'left');
$freRE->Button(-text => 'etaltml_RDS_AFSearch_start', -command => sub { 
	$RDSAFSearchData_AFList = $RDSAFSearchData_AFListStr;
	$RDSAFSearchData_AFList =~ s/[,\s]+/ /g;
	$RDSAFSearchData_AFList =~ s/^\s+//;
	$RDSAFSearchData_AFList =~ s/\s+$//;
	gui_print("eRAsta $cur_recv_idx $RDSAFSearchData_PI $RDSAFSearchData_AFList\n");
})->pack(-side => "left");
$RDSAFSearchData_PI = 0;
$freRE->Checkbutton(-text => 'PI', -variable => \$RDSAFSearchData_PI)->pack(-side=>'left');
$freRE->Label(-text => "AFList:")->pack(-side => "left");
$freRE->Entry(-textvariable => \$RDSAFSearchData_AFListStr, -width => 8)->pack(-side => "left");
$freRAsto = $nbpe[TAB_RDS_IDX]->Frame->pack;
$freRAsto->Button(-text => 'etaltml_RDS_AFSearch_stop', -command => sub { gui_print("eRAsto $cur_recv_idx\n"); })->pack(-side => "left");
$freRAsto->Button(-text => 'etaltml_RDS_seek_start', -command => sub { gui_print("eRss $cur_recv_idx $etalSeekDirectionTy_cnv{$RDSSeekStart_directionstr} $RDSSeekStart_step $RDSSeekStart_mute\n"); })->pack(-side => "left");
$RDSSeekStart_directionstr = $etalSeekDirectionTy[0];
$freRAsto->Optionmenu(-variable => \$RDSSeekStart_directionstr, -options => \@etalSeekDirectionTy)->pack(-side => "left");
$freRAsto->Label(-text => "step (kHz):")->pack(-side => "left");
$RDSSeekStart_step = 100;
$freRAsto->Entry(-textvariable => \$RDSSeekStart_step, -width => 8)->pack(-side => "left");
$RDSSeekStart_mute = 0;
$freRAsto->Checkbutton(-text => 'mute', -variable => \$RDSSeekStart_mute)->pack(-side=>'left');

# notebook page SS
##################
# Frame Seamless Estimation
$frameSE = $nbpe[TAB_SS_IDX]->Frame;

# Button etal_seamless_estimation_start
$frameSE->Button(
	-text    => 'etal_seamless_estimation_start',
	-command => sub { gui_print("esesta $cur_recv_idx $second_hRecv_idx 1 $esesta_startPos $esesta_stopPos\n"); }
)->pack(-side=>'left');

# Button etal_seamless_estimation_stop
$frameSE->Button(
	-text    => 'etal_seamless_estimation_stop',
	-command => sub { gui_print("esesto $cur_recv_idx $second_hRecv_idx\n"); }
)->pack(-side=>'left');

$frameSE->pack;

# Frame Seamless Estimation parameters
$frameSE2 = $nbpe[TAB_SS_IDX]->Frame;
$frameSE2->Label(-text => "2nd hRecv Idx:")->pack(-side => "left");
$second_hRecv_idx = 1;
$frameSE2->Optionmenu(-variable => \$second_hRecv_idx, -options => [0 .. 4])->pack(-side => "left");
# Entry start position
$frameSE2->Label(-text => 'Start pos:')->pack(-side=>'left');
$frameSE2->Entry(-textvariable => \$esesta_startPos, -width => 8)->pack(-side=>'left');
$esesta_startPos = 0;
$frameSE2->Label(-text => 'Stop pos:')->pack(-side=>'left');
$frameSE2->Entry(-textvariable => \$esesta_stopPos, -width => 8)->pack(-side=>'left');
$esesta_stopPos = -480000;

$frameSE2->pack;

# Frame Seamless Switch
$frameSS =$nbpe[TAB_SS_IDX]->Frame;

# Button etal_seamless_switching early FM
$frameSS->Button(
	-text    => 'etal_SS early FM',
	-command => sub { if ($essw_default_flag) { gui_print("essw $cur_recv_idx $second_hRecv_idx 2 0 8888888 0 0 0 1 1\n"); } else { gui_print("essw $cur_recv_idx $second_hRecv_idx 2\n"); } },
)->pack(-side=>'left');
# Button etal_seamless_switching early FM to DAB
$frameSS->Button(
	-text    => 'etal_SS early FM to DAB',
	-command => sub {
		# etal_seamless_switch
		if ($essw_default_flag) {
			gui_print("essw $cur_recv_idx $second_hRecv_idx 0 0 8888888 0 0 0 1 1\n");
		}
		else {
			gui_print("essw $cur_recv_idx $second_hRecv_idx 0\n");
		}
		# etal_audio_select
		gui_print("eas $cur_recv_idx ".ETAL_AUDIO_SOURCE_DCOP_STA660."\n");
	}
)->pack(-side=>'left');
# Button etal_seamless_switching FM
$frameSS->Button(
	-text    => 'etal_SS FM',
	-command => sub { if ($essw_default_flag) { gui_print("essw $cur_recv_idx $second_hRecv_idx 1 0 8888888 0 0 0 1 1\n"); } else { gui_print("essw $cur_recv_idx $second_hRecv_idx 1\n"); } }
)->pack(-side=>'left');

# Button etal_seamless_switching DAB
$frameSS->Button(
	-text    => 'etal_SS DAB',
	-command => sub { if ($essw_default_flag) { gui_print("essw $cur_recv_idx $second_hRecv_idx 0 0 8888888 0 0 0 1 1\n"); } else { gui_print("essw $cur_recv_idx $second_hRecv_idx 0\n"); } }
)->pack(-side=>'left');

# chekbutton default switch
$frameSS->Checkbutton(-text => 'Default SS', -variable => \$essw_default_flag)->pack(-side=>'left');

$frameSS->pack;

# notebook page SF
##################
# Frame Seamless Estimation
$fregFRFP = $nbpe[TAB_SF_IDX]->Frame->pack;
$fregFRFP->Button(-text => 'etaltml_getFreeReceiverForPath', -command => sub { gui_print("egFRFP $cur_recv_idx $EtalPathName_cnv{$vI_path}\n"); })->pack(-side=>'left');
$vI_path = $EtalPathName[0];
$fregFRFP->Optionmenu(-variable => \$vI_path, -options => \@EtalPathName)->pack(-side => "left");
$fregFRFP->Button(-text => 'etaltml_TuneOnServiceId', -command => sub { gui_print("eTOSI $EtalPathName_cnv{$vI_path} $vI_SearchedServiceID\n"); })->pack(-side=>'left');
$fregFRFP->Label(-text => "SID:")->pack(-side => "left");
$vI_SearchedServiceID = 0;
$fregFRFP->Entry(-textvariable => \$vI_SearchedServiceID, -width => 10)->pack(-side => "left");
$freASF = $nbpe[TAB_SF_IDX]->Frame->pack;
$freASF->Button(-text => 'etaltml_ActivateServiceFollowing', -command => sub { gui_print("eASF\n"); })->pack(-side=>'left');
$freASF->Button(-text => 'etaltml_DisableServiceFollowing', -command => sub { gui_print("eDSF\n"); })->pack(-side=>'left');
$freSFSER = $nbpe[TAB_SF_IDX]->Frame->pack;
$freSFSER->Button(-text => 'ETALTML_SF_SeamlessEstimationRequest', -command => sub { gui_print("eSFSER\n"); })->pack(-side=>'left');
$freSFSER->Button(-text => 'ETALTML_SF_SeamlessEstimationStop', -command => sub { gui_print("eSFSES\n"); })->pack(-side=>'left');
$freSFSSR = $nbpe[TAB_SF_IDX]->Frame->pack;
$freSFSSR->Button(-text => 'ETALTML_SF_SeamlessSwitchRequest', -command => sub { gui_print("eSFSSR $isSeamless\n"); })->pack(-side=>'left');
$isSeamless = 0;
$freSFSSR->Checkbutton(-text => 'isSeamless', -variable => \$isSeamless)->pack(-side=>'left');

# notebook page RdWr
####################
# Frame read parameter
$frerp = $nbpe[TAB_RDWR_IDX]->Frame->pack;
$frerp->Button(-text => 'etal_read_parameter', -command => sub { gui_print("erp $tuner_idx $etalReadWriteMode $param\n"); })->pack(-side => "left");
$frerp->Optionmenu(-variable => \$tuner_idx, -options => [0, 1, 2])->pack(-side => "left");
$etalReadWriteMode = fromIndex;
$frerp->Checkbutton(-text => 'Addr (not Idx)', -variable => \$etalReadWriteMode)->pack(-side=>'left');
$frerp->Label(-text => 'Param:')->pack(-side=>'left');
$param = "";
$frerp->Entry(-textvariable => \$param, -width => 8)->pack(-side=>'left');
$frerp->Button(-text => 'etal_write_parameter', -command => sub { gui_print("ewp $tuner_idx $etalReadWriteMode $param\n"); })->pack(-side => "left");

# notebook page Dbg
####################
# Frame DISS control
$fredDc = $nbpe[TAB_DBG_IDX]->Frame->pack;
$fredDc->Button(-text => 'etal_debug_DISS_control', -command => sub { gui_print("edDc $cur_recv_idx $etalChannelTy_cnv{$tuner_channel} $DISSmode $filter_index\n"); })->pack(-side => "left");
$tuner_channel = $etalChannelTy[1];
$fredDc->Optionmenu(-variable => \$tuner_channel, -options => \@etalChannelTy)->pack(-side => "left");
$DISSmode = ETAL_DISS_MODE_AUTO;
$fredDc->Checkbutton(-text => 'Manual', -variable => \$DISSmode)->pack(-side=>'left');
$fredDc->Optionmenu(-variable => \$filter_index, -options => [0 .. 9])->pack(-side => "left");
$fredDc->Button(-text => 'etal_debug_get_WSP_Status', -command => sub { gui_print("edgWS $cur_recv_idx\n"); })->pack(-side => "left");
$fredVc = $nbpe[TAB_DBG_IDX]->Frame->pack;
$fredVc->Button(-text => 'etal_debug_VPA_control', -command => sub { gui_print("edVc $cur_recv_idx $status $hReceiver_bg\n"); })->pack(-side => "left");
$status = ETAL_DISS_MODE_AUTO;
$fredVc->Checkbutton(-text => 'status', -variable => \$status)->pack(-side=>'left');
$fredVc->Label(-text => "hRecv bg Idx:")->pack(-side => "left");
$hReceiver_bg = 1;
$fredVc->Optionmenu(-variable => \$hReceiver_bg, -options => [0 .. 4])->pack(-side => "left");
$header = $headerUsed = $defaultLevel = $defaultLevelUsed = $filterNum = 0;
$filterClass = $filterMask = $filterLevel = "";
$fredVc->Button(-text => 'etal_trace_config', -command => sub { 
	$filterClass = $filterClassStr;
	$filterMask = $filterMaskStr;
	$filterLevel = $filterLevelStr;
	$filterClass =~ s/[,\s]+/ /g;
	$filterMask =~ s/[,\s]+/ /g;
	$filterLevel =~ s/[,\s]+/ /g;
	$filterClass =~ s/^\s+//;
	$filterMask =~ s/^\s+//;
	$filterLevel =~ s/^\s+//;
	$filterClass =~ s/\s+$//;
	$filterMask =~ s/\s+$//;
	$filterLevel =~ s/\s+$//;
	$filterNum = @{[$filterClass =~ /\s/g]};
	#$filterNum = map $_, $filterClass =~ /\s/gs;
	if ($filterNum != @{[$filterMask =~ /\s/g]}) {
		$mw->Dialog(-title => 'Error', -text => "mask nb not $filterNum", -bitmap => 'error', -default_button => 'ok', -buttons => ['ok'])->Show;
	}
	else {
		if ($filterNum != @{[$filterLevel =~ /\s/g]}) {
			$mw->Dialog(-title => 'Error', -text => "level nb not $filterNum", -bitmap => 'error', -default_button => 'ok', -buttons => ['ok'])->Show;
		}
		else {
			if ($filterNum > 0) {
				$filterNum++;
			}
			gui_print("etc ".sprintf("%d", !$header)." $headerUsed $defaultLevel $defaultLevelUsed $filterNum $filterClass $filterMask $filterLevel\n");
		}
	}
})->pack(-side => "left");
$fredVc->Checkbutton(-text => 'header', -variable => \$header)->pack(-side=>'left');
$fredVc->Checkbutton(-text => 'headerUsed', -variable => \$headerUsed)->pack(-side=>'left');
$fredVcp = $nbpe[TAB_DBG_IDX]->Frame->pack;
$fredVcp->Label(-text => "defaultLevel:")->pack(-side => "left");
$fredVcp->Optionmenu(-variable => \$defaultLevel, -options => [0 .. 3])->pack(-side => "left");
$fredVcp->Checkbutton(-text => 'defaultLevelUsed', -variable => \$defaultLevelUsed)->pack(-side=>'left');
$fredVcp->Label(-text => 'filter class:')->pack(-side=>'left');
$fredVcp->Entry(-textvariable => \$filterClassStr, -width => 8)->pack(-side=>'left');
$fredVcp->Label(-text => 'mask:')->pack(-side=>'left');
$fredVcp->Entry(-textvariable => \$filterMaskStr, -width => 8)->pack(-side=>'left');
$fredVcp->Label(-text => 'level:')->pack(-side=>'left');
$fredVcp->Entry(-textvariable => \$filterLevelStr, -width => 8)->pack(-side=>'left');

# notebook page SCP
###################
# Frame start SCP
$frescp = $nbpe[TAB_SCP_IDX]->Frame->pack;
$frescp->Button(-text => 'Etal Log to scp', -command => sub { 
	$fileEtalLog = $mw->FBox(-type => 'open')->Show;
	if (-e "$fileEtalLog")
	{
		$fileEtalLogScp = "${fileEtalLog}.scp";
		etalLod2Cmd_convert("$fileEtalLog", "$fileEtalLogScp");
	}
})->pack(-side => "left");
$filescp = "";
$frescp->Button(-text => 'Load scp', -command => sub { $filescp = $mw->FBox(-type => 'open')->Show; })->pack(-side => "left");
$BtnStartScp = $frescp->Button(-text => 'Start scp', -command => sub { #$scpExecAbortReq = 0; scp_exec($filescp, \$scpProgress);
	if ($scpExecAbortRsp)
	{
		if (-e "$filescp")
		{
			$scpExecAbortReq = $scpExecAbortRsp = 0; 
			$BtnStartScp->configure(-state => 'disabled');
			$workerScpExecThread =threads->new(\&worker_scp_exec, "$filescp");
			#$workerScpExecThread->detach;
			$scpUpdateTimer = $mw->repeat(500,\&scpUpdateProgress);
		}
		else
		{
			$mw->Dialog(-title => 'Error', -text => "scp file not found: $filescp", -bitmap => 'error', -default_button => 'ok', -buttons => ['ok'])->Show;
		}
	}
	else
	{
		$mw->Dialog(-title => 'Error', -text => "unable to start scp, may be worker_scp_exec thread is running", -bitmap => 'error', -default_button => 'ok', -buttons => ['ok'])->Show;
	}
});
$BtnStartScp->pack(-side => "left");
$frescp->ProgressBar(-anchor => 'w', -from => 0, -to => 100, -length => 100, -variable => \$scpProgress, -width => 16, -colors => [0 => 'green'], -blocks => 0)->pack(-side=>'left');
$frescp->Button(-text => 'Stop scp', -command => sub { $scpExecAbortReq = 1; })->pack(-side => "left");
$frescpfn = $nbpe[TAB_SCP_IDX]->Frame->pack;
$frescpfn->Label(-text => "Scp file:")->pack(-side => "left");
$frescpfn->Entry(-textvariable => \$filescp, -width => 48)->pack(-side => "left");

# show gui widget
MainLoop;
