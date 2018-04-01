import qbs 1.0

Project {
    name: "dog"

    Application {
        name: "dog"
        files: [
            "kernel.cpp",
            "kernel.h",
            "main.cpp",
            "resources.qrc",
            "logger.cpp",
            "logger.h"
        ]
        Depends { name: "cpp" }
        Depends { name: "Qt.widgets" }
        Depends { name: "doglib" }

        Group {
            qbs.install: true
            qbs.installDir: ""
            fileTagsFilter: "application"
        }
    }

    DynamicLibrary {
        name: "doglib"
        Depends { name: "cpp" }
        Depends { name: "Qt.widgets" }
        files: [
            "PluginInterface.h",
            "PluginInterface.cpp"
        ]
        Group {
            qbs.install: true
            qbs.installDir: ""
            fileTagsFilter: "dynamiclibrary"
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
        Depends { name: "doglib" }
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
        Depends { name: "doglib" }
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
            "plugins/builddircleaner/builddircleaner.cpp"
        ]
        Depends { name: "cpp" }
        Depends { name: "Qt.widgets" }
        Depends { name: "doglib" }
        cpp.includePaths: [ '.' ]
        Group {
            qbs.install: true
            qbs.installDir: "plugins/"
            fileTagsFilter: "dynamiclibrary"
        }
    }
    DynamicLibrary {
        name: "backup"
        files: [
            "plugins/backup/backup.h",
            "plugins/backup/backup.cpp"
        ]
        Depends { name: "cpp" }
        Depends { name: "Qt.widgets" }
        Depends { name: "doglib" }
        cpp.includePaths: [ '.' ]
        Group {
            qbs.install: true
            qbs.installDir: "plugins/"
            fileTagsFilter: "dynamiclibrary"
        }
    }
}

