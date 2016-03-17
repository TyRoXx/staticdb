from conans import ConanFile
import os

class StaticDbConan(ConanFile):
    name = "staticdb"
    version = "0.1"
    generators = "cmake"
    requires = "silicium/0.9@TyRoXx/master", "sqlite3/3.10.2@TyRoXx/stable"
    url="http://github.com/tyroxx/staticdb"
    license="MIT"

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dylib*", dst="bin", src="lib")
