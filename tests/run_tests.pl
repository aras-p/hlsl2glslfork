#!/usr/bin/perl -w

use warnings;
use strict;
use Digest::SHA1;

use FindBin qw($Bin);
use File::Basename;
my $testdir = $Bin;
undef $Bin;
#print("testdir is $testdir\n");


my $GPrintCmds = 0;

my @modules = qw( fragment );


sub compare_files {
    my ($a, $b, $endlines) = @_;

    if (not open(FILE1, '<', $a)) {
        return (0, "Couldn't open '$a' for checksum");
    }
    if (not open(FILE2, '<', $b)) {
        close(FILE1);
        return (0, "Couldn't open '$b' for checksum");
    }

    my $sha1 = Digest::SHA1->new;
    my $sha2 = Digest::SHA1->new;

    if (not $endlines) {
        $sha1->addfile(*FILE1);
        $sha2->addfile(*FILE2);
    } else {
        while (<FILE1>) { s/[\r\n]//g; $sha1->add($_); }
        while (<FILE2>) { s/[\r\n]//g; $sha2->add($_); }
    }

    close(FILE1);
    close(FILE2);

    if ($sha1->hexdigest ne $sha2->hexdigest) {
        return (0, "Result doesn't match expectations");
    }

    return (1);
}

my %tests = ();

$tests{'output'} = sub {
    my ($module, $fname) = @_;
	my ($basename) = $fname =~ /(.*).txt/;
    my $desired = $basename . '-corr.txt';
    my $output = 'out-' . basename($fname);
    my $cmd = undef;
    my $endlines = 1;

    # !!! FIXME: this should go elsewhere.
    if ($module eq 'fragment') {
        $cmd = "./hlsl2glsltest/hlsl2glsltest f '$fname' '$output'";
    } else {
        return (0, "Don't know how to do this module type");
    }
    $cmd .= ' 2>/dev/null 1>/dev/null';

    print("$cmd\n") if ($GPrintCmds);

    if (system($cmd) != 0) {
        unlink($output) if (-f $output);
        die "foo";
        return (0, "External program reported error");
    }

    if (not -f $output) { return (0, "Didn't get any output file"); }

    my @retval = compare_files($desired, $output, $endlines);
    #unlink($output);
    return @retval;
};

$tests{'errors'} = sub {
    return (-1, 'Unimplemented');  # !!! FIXME
};

my $totaltests = 0;
my $pass = 0;
my $fail = 0;
my $skip = 0;

my $result = '';
foreach (@modules) {
    my $module = $_;
    foreach (keys %tests) {
        my $testtype = $_;
        my $fn = $tests{$_};
        my $d = "$testdir/$module/$testtype";
        next if (not -d $d);  # no tests at the moment.
        opendir(TESTDIR, $d) || die("Failed to open dir '$d': $!\n");
        print(" ... $module / $testtype ...\n");
        my $fname = readdir(TESTDIR);
        while (defined $fname) {
            my $origfname = $fname;
            $fname = readdir(TESTDIR);  # set for next iteration.
            next if (-d $origfname);
            next if ($origfname =~ /\-corr.txt\Z/);
            next if ($origfname =~ /\-out.txt\Z/);
            next if ($origfname =~ /^\./);
            my $fullfname = "$d/$origfname";
            my ($rc, $reason) = &$fn($module, $fullfname);
            if ($rc == 1) {
                $result = 'PASS';
                $pass++;
            } elsif ($rc == 0) {
                $result = 'FAIL';
                $fail++;
            } elsif ($rc == -1) {
                $result = 'SKIP';
                $skip++;
            }

            if (defined $reason) {
                $reason = " ($reason)";
            } else {
                $reason = '';
            }
            print("$result ${origfname}${reason}\n");

            $totaltests++;
        }
        closedir(TESTDIR);
    }
}

print("\n$totaltests tests, $pass passed, $fail failed, $skip skipped.\n\n");

exit(($fail > 0) ? 1 : 0);

# end if run_tests.pl ...

