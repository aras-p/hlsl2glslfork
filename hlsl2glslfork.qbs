import qbs 1.0
import qbs.File
import qbs.FileInfo

Project {

    FileTagger {
        pattern: "*.l"
        fileTags: ["lex"]
    }

    FileTagger {
        pattern: "*.y"
        fileTags: ["bison"]
    }

    Rule {
        /*
        %(RootDir)%(Directory)..\..\tools\flex.exe %(FullPath)
        if EXIST %(RootDir)%(Directory)gen_hlslang.cpp del %(RootDir)%(Directory)gen_hlslang.cpp
        move gen_hlslang.cpp %(RootDir)%(Directory)gen_hlslang.cpp
        cd %(RootDir)%(Directory)
        rename gen_hlslang.cpp Gen_hlslang.cpp
        */
        id: flex
        inputs: ["lex"]
        Artifact {
            fileName: product.sourceDirectory + "/Generated/" + input.baseDir + "/Gen_" + input.baseName + ".cpp"
            fileTags: ["cpp"]
        }

/* -- There's a problem with flex where "%option outfile" in the input overrides -oFILE on the command line.
      I wanted to work around this by using the -t option instead (output to stdout), but ran into lots of
      problems writing the stdoutFilterFunction that I'd need to use to write the file. No matter what I did
      (stdoutFilterFunction as a string or as a function) I ran into:
      "ERROR: Error when calling output filter function: ReferenceError: Can't find variable: TextFile"
      On the #qt-qbs IRC channel, ckandeler suggested moving it with a second command instead, which is what
      I've implemented.

        prepare: {
//            var args = ["-o" + output.fileName]; // This doesn't work if the .l file contains "%option outfile="
            var args = ["-t"];                       // .. so use this to write the file to stdout instead.
            args.push(input.fileName);
            var commandName = product.moduleProperty("qbs", "targetOS").contains('windows') ? product.sourceDirectory + "/tools/flex.exe" : "flex";
            var cmd = new Command(commandName, args);
//            cmd.stdoutFilterFunction  = "function(output) {";
//            cmd.stdoutFilterFunction += '  var file = new TextFile("' + output.fileName + '",' + TextFile.WriteOnly + ');'
//            cmd.stdoutFilterFunction += '  var file = new TextFile("' + output.fileName + '",TextFile.WriteOnly);'
//            cmd.stdoutFilterFunction += '  var file = new TextFile(output.fileName, TextFile.WriteOnly);'
//            cmd.stdoutFilterFunction += "  file.write(output);";
//            cmd.stdoutFilterFunction += "  file.close();";
//            cmd.stdoutFilterFunction += "}";
            cmd.stdoutFilterFunction = function(standardout) {
                var of = new TextFile(output.fileName, TextFile.WriteOnly);
                of.write(standardout);
                of.close();
                return;
            }
            return cmd;
        }
*/
        prepare: {
            var commands = [];
            var args = [input.fileName];
            var flexCommandName = product.moduleProperty("qbs", "targetOS").contains('windows') ? product.sourceDirectory + "/tools/flex.exe" : "flex";
            var cmd = new Command(flexCommandName, args);
            cmd.description = 'lexxing ' + FileInfo.fileName(input.fileName)
            commands.push(cmd);
            var cmd = new JavaScriptCommand();
            cmd.src = "Gen_hlslang.cpp";
            cmd.dst = output.fileName;
            cmd.sourceCode = function() {
                File.copy(src, dst);
                File.remove(src);
            }
            cmd.description = 'moving ' + cmd.src + ' to ' + cmd.dst;
            commands.push(cmd);
            return commands;
        }
    }

    Rule {
        /*
        SET BISON_SIMPLE=%(RootDir)%(Directory)..\..\tools\bison.simple
        SET BISON_HAIRY=%(RootDir)%(Directory)..\..\tools\bison.simple
        cd %(RootDir)%(Directory)
        %(RootDir)%(Directory)..\..\tools\bison.exe -d -t -v %(Filename).y
        if EXIST %(RootDir)%(Directory)Gen_%(Filename)_tab.cpp del %(RootDir)%(Directory)Gen_%(Filename)_tab.cpp
        rename %(RootDir)%(Directory)%(Filename)_tab.c Gen_%(Filename)_tab.cpp
        */
        id: bison
        inputs: ["bison"]

        Artifact {
            fileName: product.sourceDirectory + "/Generated/" + input.baseDir + "/Gen_" + input.baseName + "_tab.cpp"
            fileTags: ["cpp"]
        }

        Artifact {
            fileName: product.sourceDirectory + "/Generated/" + input.baseDir + "/" + input.baseName + "_tab.h"
            fileTags: ["h"]
        }

        prepare: {
            var commands = [];

            var args = [input.fileName, "-d", "-t", "-v"];
            args.push("--output=" + FileInfo.path(input.fileName) + "/hlslang_tab.c");
            var commandName = product.moduleProperty("qbs", "targetOS").contains('windows') ? product.sourceDirectory + "/tools/bison.exe" : "bison";
            var cmd = new Command(commandName, args);
            cmd.environment.push("BISON_SIMPLE=" + product.sourceDirectory + "/tools/bison.simple");
            cmd.environment.push( "BISON_HAIRY=" + product.sourceDirectory + "/tools/bison.simple");
            cmd.description = 'bisoning ' + FileInfo.fileName(input.fileName)
            commands.push(cmd);

            var cmd = new JavaScriptCommand();
            var output = outputs["cpp"][0]
            cmd.src = FileInfo.path(input.fileName) + "/hlslang_tab.c";
            cmd.dst = output.fileName;
            cmd.sourceCode = function() {
                File.copy(src, dst);
                File.remove(src);
            }
            cmd.description = 'moving ' + cmd.src + ' to ' + cmd.dst;
            commands.push(cmd);

            var cmd = new JavaScriptCommand();
            var output = outputs["h"][0]
            cmd.src = FileInfo.path(input.fileName) + "/hlslang_tab.h";
            cmd.dst = output.fileName;
            cmd.sourceCode = function() {
                File.copy(src, dst);
                File.remove(src);
            }
            cmd.description = 'moving ' + cmd.src + ' to ' + cmd.dst;
            commands.push(cmd);

            return commands;
        }
    }

    StaticLibrary {
        name: "hlsl2glsl"
        /*
        Changing maintainerMode from true to false requires
        that you remove the old build folder, otherwise you
        will get errors such as:
        evaluating prepare script: TypeError: Result of expression 'config' [undefined] is not an object.'
        */
        property bool maintainerMode: true
        property string OSDepPath: {
            if (qbs.targetOS.contains("windows"))
                return "Windows";
            else if (qbs.targetOS.contains("linux"))
                return "Linux";
            else if (qbs.targetOS.contains("darwin"))
                return "Mac";
            else
                return "UnknownOS";
        }
        Group {
            name: "OSDependent-Windows"
            condition: qbs.targetOS.contains("windows")
            files: ["hlslang/OSDependent/Windows/ossource.cpp",
                    "hlslang/OSDependent/Windows/osinclude.h"]
        }
        Group {
            name: "OSDependent-Linux"
            condition: qbs.targetOS.contains("linux")
            files: ["hlslang/OSDependent/Linux/ossource.cpp",
                    "hlslang/OSDependent/Linux/osinclude.h"]
        }
        Group {
            name: "OSDependent-Mac"
            condition: qbs.targetOS.contains("darwin")
            files: ["hlslang/OSDependent/Mac/ossource.cpp",
                    "hlslang/OSDependent/Mac/osinclude.h"]
        }
        Group {
            name: "GLSLCodeGen"
            files: ["hlslang/GLSLCodeGen/*.cpp",
                    "hlslang/GLSLCodeGen/*.h"]
        }
        Group {
            name: "MachineIndependent"
            files: ["hlslang/MachineIndependent/**/*.c",
                    "hlslang/MachineIndependent/**/*.cpp",
                    "hlslang/MachineIndependent/**/*.h"]
        }
        Group {
            name: "GrammarAndParserSource"
            condition: maintainerMode
            files: {
                if (maintainerMode) {
                    return ["hlslang/MachineIndependent/hlslang.l",
                            "hlslang/MachineIndependent/hlslang.y"];
                }
                return [];
            }
        }

        Group {
            name: "GrammarAndParserArtefacts"
            condition: !maintainerMode
            files: {
                if (!maintainerMode) {
                    return ["Generated/hlslang/MachineIndependent/Gen_hlslang.cpp",
                            "Generated/hlslang/MachineIndependent/Gen_hlslang_tab.cpp",
                            "Generated/hlslang/MachineIndependent/hlslang_tab.h"];
                }
                return [];
            }
        }

        Group {
            name: "Include"
            files: ["hlslang/Include/*.h"]
        }
        Depends { name: "cpp" }
        cpp.includePaths: base.concat(["hlslang",
                                       "hlslang/Include",
                                       "hlslang/MachineIndependent",
                                       "hlslang/OSDependent/"+OSDepPath,
                                       "Generated/hlslang/MachineIndependent"])
        Properties {
            condition: !qbs.toolchain.contains("msvc")
            // Annoyingly, a bug in flex forces -Wno-sign-compare into this list:
            // http://sourceforge.net/p/flex/bugs/140/
            // It may be possible to isolate this flag specifically to Gen_hlslang.cpp?
            cpp.commonCompilerFlags: base.concat(["-Wno-unused-parameter", "-Wno-sign-compare", "-Werror"])
        }
        Properties {
            condition: qbs.toolchain.contains("msvc")
            cpp.commonCompilerFlags: base.concat(["/WX"])
        }
    }

    Application {
        name: "hlsl2glsltest"
        type: "application"
        Depends { name: "hlsl2glsl" }
        Depends { name: "cpp" }
        property stringList StaticLibs: {
            if (qbs.targetOS.contains("windows"))
                return ["opengl32", "Gdi32.lib", "User32.lib"];
            else if (qbs.targetOS.contains("linux"))
                if (qbs.toolchain.contains("clang"))
                    return ["GLEW", "GLU", "GL", "glut", "pthread"];
                else
                    return ["GLEW", "GLU", "GL", "glut"];
            else
                return [];
        }
        property stringList Frameworks: {
            if (qbs.targetOS.contains("darwin"))
                return ["OpenGL", "GLUT"];
        }

        files: ["tests/hlsl2glsltest/hlsl2glsltest.cpp"]
        cpp.includePaths: base.concat(["hlslang/Include"])
        cpp.staticLibraries: base.concat(StaticLibs)
        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.frameworks: outer.concat(Frameworks)
        }
    }
}
