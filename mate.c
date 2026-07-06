#define MATE_IMPLEMENTATION
#include "mate.h"

#define OUTPUT_TARGET "my-editor"
#define INCLUDE_DIR "include/"
#define SOURCE_DIR "source/"

// act like you didn't see this
// (setq org-download-image-dir (expand-file-name "./assets/"))

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
		StringBuilder flags = SBCreate(mate_state.arena);
		SBAddF(&flags, "%s -Wno-unused-command-line-argument", getenv("CLINK_FLAGS"));

		Executable exe = CreateExecutable((ExecutableOptions) {
			.output = OUTPUT_TARGET,
			.warnings = is_release_build ? FLAG_WARNINGS_NONE : FLAG_WARNINGS_VERBOSE,
			.debug = is_release_build ? 0 : FLAG_DEBUG,
			.optimization = is_release_build ? FLAG_OPTIMIZATION_AGGRESSIVE : 0,
			.std = FLAG_STD_C11,
			.sanitizer = is_release_build ? 0 : FLAG_SANITIZER,
			.flags = flags.buffer.data,
		});

		AddFile(exe, "./source/*.c");

		AddIncludePaths(exe, INCLUDE_DIR, SOURCE_DIR);

		InstallExecutable(exe);
		CreateCompileCommands(exe);

		if (run) {
			StringBuilder sb = SBCreate(mate_state.arena);
			SBAddS(&sb, "ASAN_OPTIONS=log_path=./asan.log ");
			SBAdd(&sb, exe.outputPath);
			bool reached_dashes = false;
			for (size_t i=0; i < (size_t)argc; i += 1) {
				const char *arg = argv[i];
				if (strcmp(arg, "--") == 0) {
					reached_dashes = true;
					continue;
				}
				if (reached_dashes) {
					SBAddF(&sb, " %s", arg);
				}
			}
			int result = RunCommand(sb.buffer);
			fprintf(stderr, "Exited with %d\n", result);
		}
	}
	EndBuild();
	return 0;
}
