import pytest

def pytest_addoption(parser):
    """
        This function is used to extract input1 and input2 values from the command line
    """
    parser.addoption(
        "--file", action="store", default="file"
    )
