#define MATE_IMPLEMENTATION
#include "mate.h"

#define OUTPUT_TARGET "my-editor"
#define INCLUDE_DIR "include/"
#define SOURCE_DIR "source/"

int main(int argc, char **argv)
{
	bool run = false;
	bool is_release_build = false;

	if (argc > 1) {
		if (strcmp(argv[1], "run") == 0)     run = true;
		if (strcmp(argv[1], "debug") == 0)   is_release_build = false;
		if (strcmp(argv[1], "release") == 0) is_release_build = true;
	}

	CreateConfig((MateOptions) {
		.rebuild_flags = "-w",
	});

	StartBuildEx(argc, argv);
	{
		Executable exe = CreateExecutable((ExecutableOptions) {
			.output = OUTPUT_TARGET,
			.warnings = is_release_build ? FLAG_WARNINGS_NONE : FLAG_WARNINGS_VERBOSE,
			.debug = is_release_build ? 0 : FLAG_DEBUG,
			.optimization = is_release_build ? FLAG_OPTIMIZATION_AGGRESSIVE : FLAG_OPTIMIZATION_NONE,
			.std = FLAG_STD_C11,
			.sanitizer = is_release_build ? 0 : FLAG_SANITIZER,
		});

		AddFile(exe, "./source/*.c");

		AddIncludePaths(exe, INCLUDE_DIR);

		InstallExecutable(exe);
		CreateCompileCommands(exe);

		if (run) {
			int result = RunCommand(exe.outputPath);
			fprintf(stderr, "Exited with %d\n", result);
		}
	}
	EndBuild();
	return 0;
}
