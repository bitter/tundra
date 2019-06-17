use File::Path qw(mkpath);

if ($^O eq "MSWin32")
{
    system("bee.exe build::tundra2::release") eq 0 or die("failed building tundra");
    system("bee.exe run::tundra2-unittest::release") eq 0 or die("failed tundra tests");
} else
{
    system("mono bee.exe build::tundra2::release") eq 0 or die("failed building tundra");
    system("mono bee.exe run::tundra2-unittest::release") eq 0 or die("failed tundra tests");
}