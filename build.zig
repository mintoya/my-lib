const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // Create executable
    const exe = b.addExecutable(.{
        .name = "a.exe",
        .target = target,
        .optimize = optimize,
    });

    // Add C source file
    exe.addCSourceFile(.{
        .file = b.path("examples/alloctest.c"),
        .flags = &.{
            "-w",
            "-g",
            "-O0",
        },
    });

    // Link libc
    exe.linkLibC();

    // Platform-specific flags
    if (target.result.os.tag != .windows) {
        // Non-Windows: AddressSanitizer, dl library
        exe.addCSourceFile(.{
            .file = b.path("examples/alloctest.c"),
            .flags = &.{
                "-w",
                "-g",
                "-O0",
                "-fsanitize=address",
                "-D_GNU_SOURCE",
            },
        });
        exe.linkSystemLibrary("dl");
        // Note: -lasan is typically handled automatically by -fsanitize=address
    } else {
        // Windows: LeakSanitizer, dbghelp
        exe.addCSourceFile(.{
            .file = b.path("examples/alloctest.c"),
            .flags = &.{
                "-w",
                "-g",
                "-O0",
                "-fsanitize=leak",
            },
        });
        exe.linkSystemLibrary("dbghelp");
    }

    // Install artifact to zig-out/bin
    b.installArtifact(exe);

    // Create run step
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the application");
    run_step.dependOn(&run_cmd.step);

    // Create profile step (perf on Linux)
    if (target.result.os.tag == .linux) {
        const perf_cmd = b.addSystemCommand(&.{
            "perf", "record", 
        });
        perf_cmd.addArtifactArg(exe);
        perf_cmd.step.dependOn(b.getInstallStep());

        const profile_step = b.step("profile", "Profile with perf (Linux only)");
        profile_step.dependOn(&perf_cmd.step);
        
        // Add echo for instructions
        const echo = b.addSystemCommand(&.{"echo", "Run 'perf report' to view results"});
        echo.step.dependOn(&perf_cmd.step);
        profile_step.dependOn(&echo.step);
    }
}
