from conan import ConanFile
import os
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class xppRecipe(ConanFile):
    name = "xpp"
    version = "1.0"
    package_type = "library"

    # Optional metadata
    license = "MIT"
    author = "Robert Knutsson zybreak@gmail.com"
    url = "github.com/zybreak/xpp"
    description = "A C++23 RAII wrapper for XCB"
    topics = ("x11", "xcb")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*", "generators/*", "cmake/*"

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generator = "Ninja"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
    
    def build_requirements(self):
        self.tool_requires("cmake/[>=3.30]")
        self.tool_requires("ninja/[>=1.12]")

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["xpp"]
        self.cpp_info.set_property("cmake_file_name", "xpp")
        self.cpp_info.set_property("cmake_target_name", "xpp::xpp")
        self.cpp_info.set_property("cmake_find_mode", "none")
        self.cpp_info.builddirs.append(os.path.join("lib", "cmake", "xpp"))
