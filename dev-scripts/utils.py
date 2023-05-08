"""
Utility module
Log -- A simple colorized console wrapper
Shell -- Wrap up common linux shell utillities
"""

# standard lib
import subprocess as sp
import sys


class Log:
    """
    A convience wrapper class for formatted colorized output for linux systems only
    """

    black = "\033[1;30m"
    red = "\033[1;31m"
    grn = "\033[1;32m"
    yellow = "\033[1;33m"
    blue = "\033[1;34m"
    purp = "\033[1;35m"
    cyan = "\033[1;36m"
    white = "\033[1;37m"
    reset = "\033[0m"

    @staticmethod
    def info(msg: str):
        """
        Log a green highlighted info msg
        """
        sys.stdout.write(
            f"{Log.grn}[✓]{Log.reset}  {Log.white}{msg}{Log.reset}\n")

    @staticmethod
    def complete(msg: str):
        """
        Log a blue completion message
        """
        sys.stdout.write(
            f"{Log.blue}[✓]{Log.reset}  {Log.white}{msg}{Log.reset}\n")

    @staticmethod
    def warn(msg: str):
        """
        Log a yellow highlighted warning message
        """
        sys.stdout.write(
            f"{Log.yellow}[!]{Log.reset}  {Log.white}{msg}{Log.reset}\n")

    @staticmethod
    def error(msg: str):
        """
        Log an error message then exit the program
        """
        sys.stderr.write(
            f"{Log.red}[x]{Log.reset}  {Log.white}{msg}{Log.reset}\n")


class Shell:
    """
    @description: A wrapper around subprocess
    """

    def __init__(self):
        self.__user_shell: str = self.execute_with_output("command -v bash")

    def execute_with_output(self, command: str) -> str:
        """
        @desc get the output from a command
        @returns the output of a shell command, it will be empty if
        the command fails
        """
        output = ""
        try:
            output = sp.run(command, check=False, shell=True,
                            capture_output=True).stdout.decode().strip('\n')
        except sp.CalledProcessError as error:
            Log.error(f"{error}")

        return output

    def execute(self, command: str) -> bool:
        """
        @description: Execute a shell command
        @returns: True if the command succeeds, False otherwise
        """
        Log.info(f"Running => {command}")
        try:
            sp.run(command, check=False, shell=True,
                   executable=self.__user_shell)
        except sp.CalledProcessError as error:
            Log.error(f"{error}")
            return False

        return True

    def is_installed(self, command: str) -> bool:
        """
        @description: Check if a program is installed on the system, will only work for software
        that is in the users PATH, uses the POSIX compliant command -v, rather than which
        @returns: True, False
        """
        cmd = f"command -v {command}"
        ret: sp.CompletedProcess = sp.run(
            cmd, check=False, capture_output=True, shell=True, executable=self.__user_shell
        )
        if ret.returncode != 0:
            # the program is not installed
            return False

        return True
