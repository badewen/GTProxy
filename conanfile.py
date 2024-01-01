from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy

class GTProxyRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    default_options = {
        "libressl*:shared": True
    }

    def requirements(self):
        self.requires("cpp-httplib/0.14.3")
        self.requires("fmt/10.0.0")

        if self.settings.os != "Android":
            self.requires("libressl/3.5.3")
        else:
            self.requires("openssl/3.2.0")

        self.requires("magic_enum/[~0.8]")
        self.requires("nlohmann_json/[~3.11]")
        self.requires("pcg-cpp/cci.20210406")
        self.requires("spdlog/1.12.0")
        self.requires("sigslot/1.2.2")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        for dep in self.dependencies.values():
            if dep.cpp_info.bindirs:
                if self.settings.os == "Linux" or self.settings.os == "Android":
                    copy(self, "*.so", dep.cpp_info.bindirs[0], self.build_folder)
                elif self.settings.os == "Windows":
                    copy(self, "*.dll", dep.cpp_info.bindirs[0], self.build_folder)
            if dep.cpp_info.libdirs:
                if self.settings.os == "Linux" or self.settings.os == "Android":
                    copy(self, "*.a", dep.cpp_info.bindirs[0], self.build_folder)
                elif self.settings.os == "Windows":
                    copy(self, "*.lib", dep.cpp_info.bindirs[0], self.build_folder)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
