use File::Path qw(mkpath);

if ($^O eq "linux")
{
  $ENV{"CXX"} = "g++";
  $ENV{"CC"} = "gcc";
}

if ($^O eq "MSWin32")
{
    system("msbuild vs2017\\Tundra.sln /P:Configuration=Release /P:Platform=x64") eq 0 or die("failed msbuild");
    mkpath("artifacts/$^O_x64");
    system("copy vs2017\\x64\\Release\\tundra2.exe artifacts\\$^O_x64") eq 0 or die("failed copy");

    system("msbuild vs2017\\Tundra.sln /P:Configuration=Release /P:Platform=ARM64") eq 0 or die("failed msbuild");
    mkpath("artifacts/$^O_arm64");
    system("copy vs2017\\ARM64\\Release\\tundra2.exe artifacts\\$^O_arm64") eq 0 or die("failed copy");
} else
{
    mkpath("artifacts/$^O");
    system("make") eq 0 or die("failed make");
    system("build/t2-unittest") eq 0 or die("running unit tests failed");
    system("cp build/tundra2 artifacts/$^O/") eq 0 or die("failed copy");
}


