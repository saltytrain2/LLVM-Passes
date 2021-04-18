import os
import sys
import getopt
import subprocess
import tempfile
import string
import os
import os.path
import pytest

@pytest.mark.invar
def test_address_match(get_file, get_addr, capfd):
    os.system(fr"{get_file}")
    out, err = capfd.readouterr()
    assert out == get_addr

@pytest.fixture()
def get_addr(capfd):
    os.system("mutants/exampleprograms/addrInvariant1_8_add")
    out, err = capfd.readouterr()
    return out

@pytest.fixture()
def get_file(request):
    return request.config.getoption("--file")