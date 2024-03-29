import os
import sys
import getopt
import subprocess
import tempfile
import string
import os
import os.path
import pytest

@pytest.mark.digit
def test_binary_0(get_file):
    assert os.system(fr"{get_file} 0") == 0

@pytest.mark.digit
def test_binary_1(get_file):
    assert os.system(fr"{get_file} 1") == 0

@pytest.mark.digit
def test_binary_2(get_file):
    assert os.system(fr"{get_file} 2") == 0

@pytest.mark.digit
def test_binary_3(get_file):
    assert os.system(fr"{get_file} 3") == 0

@pytest.mark.digit
def test_binary_4(get_file):
    assert os.system(fr"{get_file} 4") == 0

@pytest.mark.digit
def test_binary_5(get_file):
    assert os.system(fr"{get_file} 5") == 0

@pytest.mark.digit
def test_binary_6(get_file):
    assert os.system(fr"{get_file} 6") == 0

@pytest.mark.digit
def test_binary_7(get_file):
    assert os.system(fr"{get_file} 7") == 0

@pytest.mark.digit
def test_binary_8(get_file):
    assert os.system(fr"{get_file} 8") == 0

@pytest.mark.digit
def test_binary_9(get_file):
    assert os.system(fr"{get_file} 9") == 0

@pytest.mark.digit
def test_binary_LargeInt(get_file):
    assert os.system(fr"{get_file} 21474836") == 0
    
@pytest.mark.digit
def test_binary_INT_MAX(get_file):
    assert os.system(fr"{get_file} 2147483647") == 0

@pytest.mark.digit
def test_binary_LONGINT_MAX(get_file):
    assert os.system(fr"{get_file} 9,223,372,036,854,775,807") == 0

@pytest.fixture()
def get_file(request):
    return request.config.getoption("--file")