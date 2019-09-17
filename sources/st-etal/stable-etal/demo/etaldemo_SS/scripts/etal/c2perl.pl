#!perl
# convert C code into perl code
use strict;
use warnings;
use Getopt::Std;
use vars qw($opt_h $opt_m $opt_o $opt_p);

my ($filenamec, $filename, $filenamepl, $filenamepm);
my ($line_no_comment, $comment_singleline, $comment_multiline, $comment_doxygen);
my ($enum, $enum_nb, $tmp, $i, $typedef, $typedef_brace);
my (@constList);

# Main function:
#################

# read command line parameters
getopts("hmop:");
if (($#ARGV >= 0) || (! $opt_h))
{
    $filenamec = $ARGV[0];
	$filename = $filenamec;
	$filename =~ s/\.[ch][p]*$//i;
	$filenamepm = "$filename.pm";
	$filenamepl = "$filenamec.pl";
}
else
{
	print "c2perl usage:\nperl c2perl.pl [options] [-p prefix] [.h file]\nit convert .h file to a perl script file .pl or module file .pm\n";
	print "  -h  show this help                -m  create a .pm module\n";
	print "  -o  module export optional        -p  module name prefix following\n";
	exit(0);
}

$comment_multiline = 0;
$comment_doxygen = 0;
$enum = 0;
$enum_nb = 0;
$typedef = 0;
$typedef_brace = 0;
if (!defined($opt_p))
{
	$opt_p = "";
}

open(HCF, "<", "$filenamec") or die "cannot open $filenamec\n";
open(HPLF, ">", "$filenamepl") or die "cannot open $filenamepl\n";
#open(HPMF, ">", "$filenamepm") or die "cannot open $filenamepm\n";
## add package begin
#print HPMF "package $filename;\nuse strict;\nuse warnings;\n\n";
while (<HCF>)
{
	$line_no_comment = $_;
	# convert comment
	# single line comment
	$comment_singleline = 0;
	if ((/\/\*.*\*\//) || (/\/\//))
	{
		$comment_singleline = 1;
		if (/\/\*\!/)
		{
			s/\/\*\!(.*)\*\//#**$1/g;
		}
		else
		{
			s/\/\*(.*)\*\//#$1/g;
		}
		s/\/\//#/;
		$line_no_comment =~ s/\/\/.*//;
		$line_no_comment =~ s/\/\*.*\*\///g;
	}
	else
	{
		# multiple line comment
		if (/\/\*/)
		{
			$comment_multiline = 1;
			$line_no_comment =~ s/\/\*.*//;
		}
		if ($comment_multiline == 1)
		{
			if ((/\/\*/) && (/\/\*\!/))
			{
				$comment_doxygen = 1;
			}
			if ($comment_doxygen == 1)
			{
				s/\/\*\!/#**/g;
			}
			else
			{
				s/\/\*/#/g;
			}
			if (/\*\//)
			{
				if ($comment_doxygen == 1)
				{
					s/^(\s*)(.*)\*\//$1#*$2/;
				}
				else
				{
					s/^(\s*)(.*)\*\//$1#$2/;
				}
				$line_no_comment =~ s/.*\*\///;
				$comment_multiline = 0;
				$comment_doxygen = 0;
			}
			else
			{
				s/^(\s*)\*/$1#/;
				$line_no_comment = "";
			}
		}
	}

#	if (($comment_singleline == 0) && ($comment_multiline == 0))
	if ($comment_multiline == 0)
	{
		if (/typedef\s+enum/)
		{
			$enum = 1;
			$enum_nb = 0;
			s/typedef/use/;
			s/enum/constant/;
		}
		elsif (/typedef\s+(struct|union)[^;]*$/)
		{
			$typedef = 1;
			$typedef_brace = 0;
		}
		# convert enum in use constant
		if ($enum == 1)
		{
			s/^(\s*[^\s]+\s*=)/$1\>/;
			if (/\s*[^\s]+\s*=\>\s*([\dxa-fA-F]+).*$/)
			{
				$tmp = $1;
				if ($tmp =~ /^0x/i)
				{
					$enum_nb = hex($tmp) + 1;
				}
				else
				{
					$enum_nb = $tmp + 1;
				}
			}
			elsif ($line_no_comment =~ /,/)
			{
				s/,/=> $enum_nb,/;
				s/($)/# warning generated value$1/;
				$enum_nb++;
			}
			elsif ((/\}.*;/) || (/use constant/) || (/\{/))
			{
				# nothing to do here
			}
			elsif (! /^\s*#/)
			{
				s/^(\s*[^\s]+)/$1 => $enum_nb # warning generated value/;
				$enum_nb++;
			}
			if (/\}.*;/)
			{
				s/\}(.*);/\}; #$1/;
				$enum = 0;
				$enum_nb = 0;
			}
		}

		# comment typedef struct or union
		if ($typedef != 0)
		{
			# count brace { }
			while ($line_no_comment =~ /\{/)
			{
				$typedef_brace++;
				$line_no_comment =~ s/\{//;
			}
			while ($line_no_comment =~ /\}/)
			{
				$typedef_brace--;
				if ($typedef_brace < 0)
				{
					die "\nERROR in typedef braces\n";
				}
				$line_no_comment =~ s/\}//;
			}
			if (! /^\s*#/)
			{
				s/^(\s*[^\s])/#$1/;
			}
			if (($typedef_brace == 0) && (/\}[^;]*;/))
			{
				$typedef = 0;
			}
		}

		# remove cast
		s/\(\s*t[FSU]\d+\s*\)//g;
		s/\(\s*ETAL_HANDLE\s*\)//g;
		s/\(\s*ETAL_HINDEX\s*\)//g;
		s/\(\s*tBool\s*\)//g;

		# convert #define in use constant
		if (/^(\s*)#define\s+[^\s\(]+\s+[^\s]+/)
		{
			s/^(\s*)#define(\s+[^\s]+\s+)/$1use constant$2 =>/;
			s/$/;/;
			s/\(//g;
			s/\)//g;
		}

		# convert typedef
		s/^(\s*typedef.*;)/#$1/;

		# fill list of constant
		if (/^\s*([^\s=]+)\s*=\>/)
		{
			push @constList, "$1";
	}
		if (/^\s*use\s+constant\s+([^\s=]+)\s*=\>/)
		{
			push @constList, "$1";
		}
	}
	print HPLF "$_";
}
close(HCF);
close(HPLF);

if ($opt_m)
{
	open(HPLF, "<", "$filenamepl") or die "cannot open $filenamepl\n";
	open(HPMF, ">", "$filenamepm") or die "cannot open $filenamepm\n";

	# add package begin
	print HPMF "package $opt_p$filename;\nuse strict;\nuse warnings;\nuse Exporter;\n";
	print HPMF 'use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);'."\n\n";
	print HPMF 'our @ISA = qw(Exporter);'."\n";
	if ($opt_o)
	{
		print HPMF "our \@EXPORT_OK = qw(";
	}
	else
	{
		print HPMF "our \@EXPORT = qw(";
	}

	# export constant
	for($i = 0; $i <= $#constList; $i++)
	{
		if (($i % 8) == 0)
		{
			print HPMF "\n   ";
		}
		print HPMF " $constList[$i]";
	}
	print HPMF "\n    );\n\%EXPORT_TAGS = ( ALL => [ \@EXPORT_OK, \@EXPORT ] );\n\n";

	# copy file .pl into .pm
	while (<HPLF>)
	{
		print HPMF "$_";
	}

	# add package end
	print HPMF "\n1;\n";

	close(HPLF);
	close(HPMF);
}
