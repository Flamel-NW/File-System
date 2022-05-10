#!/usr/bin/perl
use 5.16.0;
use warnings FATAL => 'all';

use Test::Simple tests => 31;
use IO::Handle;

sub mount {
    system("(make mount 2>&1) >> test.log &");
    sleep 1;
}

sub unmount {
    system("(make unmount 2>&1) >> test.log");
}

sub write_text {
    my ($name, $data) = @_;
    open my $fh, ">", "mnt/$name" or return;
    $fh->say($data);
    close $fh;
}

sub read_text {
    my ($name) = @_;
    open my $fh, "<", "mnt/$name" or return "";
    local $/ = undef;
    my $data = <$fh> || "";
    close $fh;
    $data =~ s/\s*$//;
    return $data;
}

sub read_text_slice {
    my ($name, $count, $offset) = @_;
    open my $fh, "<", "mnt/$name" or return "";
    my $data;
    seek $fh, $offset, 0;
    read $fh, $data, $count;
    close $fh;
    return $data;
}

system("rm -f data.nufs test.log");

say "#           == Basic Tests ==";
mount();

my $msg0 = "hello, one";
write_text("one.txt", $msg0);
ok(-e "mnt/one.txt", "File1 exists.");
ok(-f "mnt/one.txt", "File1 is regular file.");
my $msg1 = read_text("one.txt");
say "# '$msg0' eq '$msg1'?";
ok($msg0 eq $msg1, "Read back data1 correctly.");

my $msg2 = "hello, two";
write_text("two.txt", $msg2);
ok(-e "mnt/two.txt", "File2 exists.");
ok(-f "mnt/two.txt", "File2 is regular file.");
my $msg3 = read_text("two.txt");
say "# '$msg0' eq '$msg1'?";
ok($msg2 eq $msg3, "Read back data2 correctly.");

my $files = `ls mnt`;
ok($files =~ /one\.txt/, "one.txt is in the directory");
ok($files =~ /two\.txt/, "two.txt is in the directory");

my $long0 = "=This string is fourty characters long.=" x 50;
write_text("2k.txt", $long0);
my $long1 = read_text("2k.txt");
ok($long0 eq $long1, "Read back long correctly.");

my $long2 = read_text_slice("2k.txt", 10, 50);
my $right = "ng is four";
ok($long2 eq $right, "Read with offset & length");

unmount();

(!-e "mnt/one.txt") or die "one.txt exists after umount; FS never mounted?";

$files = `ls mnt`;
ok($files !~ /one\.txt/, "one.txt is not in the directory");
ok($files !~ /two\.txt/, "two.txt is not in the directory");

mount();

$files = `ls mnt`;
ok($files =~ /one\.txt/, "one.txt is in the directory still");
ok($files =~ /two\.txt/, "two.txt is in the directory still");

$msg1 = read_text("one.txt");
say "# '$msg0' eq '$msg1'?";
ok($msg0 eq $msg1, "Read back data1 correctly again.");

$msg3 = read_text("two.txt");
say "# '$msg2' eq '$msg3'?";
ok($msg2 eq $msg3, "Read back data2 correctly again.");

say "# Testing unlink...";

system("rm -f mnt/one.txt");
$files = `ls mnt`;
ok($files !~ /one\.txt/, "deleted one.txt");

unmount();

system("rm -f data.nufs test.log");

mount();

say "#           == Advanced Tests ==";

say "# Nested directories";

ok(mkdir("mnt/foo"), "Create a new directory");
ok(mkdir("mnt/tmp"), "Create another directory");
ok((-d "mnt/foo" and -d "mnt/foo"), "Directories exist");
$files = `ls mnt`;
ok(($files =~ /foo/ and $files =~ /tmp/), "Directories listed");

ok((mkdir("mnt/foo/bar") and -d "mnt/foo/bar"), "Create a nested directory");
ok((mkdir("mnt/foo/bar/baz") and -d "mnt/foo/bar/baz"), "Create a nested-nested directory");

my $msg4 = "This is a file";
write_text("tmp/file.txt", $msg4);
ok(-f "mnt/tmp/file.txt", "Create a file in a directory");
my $msg5 = read_text("tmp/file.txt");
ok($msg4 eq $msg5, "Read data back correctly");
system("mv mnt/tmp/file.txt mnt/foo");
ok(-e "mnt/foo/file.txt", "Move a file to another directory");
my $msg6 = read_text("foo/file.txt");
ok($msg4 eq $msg6, "Read data back correctly");

unmount();

system("rm -f data.nufs test.log");

mount();

say "# Large files";

say "# -> 2 blocks";

my $chunks = 256 + 128;
my $content = "1_2_3_4_5_6_7_8_" x $chunks; # $chunks * 16 bytes of data
write_text("large.txt", $content);
$files = `ls mnt`;
my $listed = $files =~ /large\.txt/;
my $exists = -f "mnt/large.txt";
my $size = -s "mnt/large.txt";
$size or $size = 0;
say "# Actual size: $size";
my $has_size = $size eq 16 * $chunks + 1;
ok(($listed and $exists and $has_size), "Large file exists and has the correct size");
my $back = read_text("large.txt");
ok($content eq $back, "Read back data from large file correctly");

say "# -> 4 blocks";
$chunks = 3 * 256 + 128;
$content = "1_2_3_4_5_6_7_8_" x $chunks; # $chunks * 16 bytes of data
write_text("larger.txt", $content);
$files = `ls mnt`;
$listed = $files =~ /larger\.txt/;
$exists = -f "mnt/larger.txt";
$size = -s "mnt/larger.txt";
$size or $size = 0;
say "# Actual size: $size";
$has_size = $size eq 16 * $chunks + 1;
ok(($listed and $exists and $has_size), "Larger file exists and has the correct size");
$back = read_text("larger.txt");
ok($content eq $back, "Read back data from larger file correctly");

unmount()

