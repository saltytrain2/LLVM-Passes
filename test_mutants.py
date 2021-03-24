import os
import sys
import getopt
import subprocess
import tempfile
import string
import pytest
import os
import os.path
import _pytest.pytester


def test_binary(get_file):
    assert os.system(f"./mutants/exampleprograms/{get_file}") == 0

@pytest.fixture()
def get_file(request):
    return request.config.getoption("--file")