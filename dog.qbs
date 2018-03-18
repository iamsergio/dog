import qbs 1.0

Project {
    name: "dog"

    Application {
        name: "dog"
        files: [
            "kernel.cpp",
            "kernel.h",
            "main.cpp",
            "PluginInterface.h",
            "resources.qrc",
            "logger.cpp",
            "logger.h"
        ]
        Depends { name: "cpp" }
        Depends { name: "Qt.widgets" }
        Group {
            qbs.install: true
            qbs.installDir: ""
            fileTagsFilter: "application"
        }
    }

    DynamicLibrary {
        name: "coredumps"
        files: [
            "plugins/coredumps/coredumps.h",
            "plugins/coredumps/coredumps.cpp"
        ]
        Depends { name: "cpp" }
        Depends { name: "Qt.widgets" }
        cpp.includePaths: [ '.' ]
        Group {
            qbs.install: true
            qbs.installDir: "plugins/"
            fileTagsFilter: "dynamiclibrary"
        }
    }
    DynamicLibrary {
        name: "modifiedgit"
        files: [
            "plugins/modifiedgit/modifiedgit.h",
            "plugins/modifiedgit/modifiedgit.cpp"
        ]
        Depends { name: "cpp" }
        Depends { name: "Qt.widgets" }
        cpp.includePaths: [ '.' ]
        Group {
            qbs.install: true
            qbs.installDir: "plugins/"
            fileTagsFilter: "dynamiclibrary"
        }
    }
    DynamicLibrary {
        name: "builddircleaner"
        files: [
            "plugins/builddircleaner/builddircleaner.h",
            "plugins/builddircleaner/builddircleaner.cpp",
            "plugins/builddircleaner/resources.qrc",
        ]
        Depends { name: "cpp" }
        Depends { name: "Qt.widgets" }
        cpp.includePaths: [ '.' ]
        Group {
            qbs.install: true
            qbs.installDir: "plugins/"
            fileTagsFilter: "dynamiclibrary"
        }
    }
}

