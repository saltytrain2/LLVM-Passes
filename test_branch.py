import os
import sys
import getopt
import subprocess
import tempfile
import string
import os
import os.path
import pytest

#Test_branch is mostly a demo on how we can evaluate stdout output using the capfd pytest fixture

# @pytest.mark.branch
# def test_branch_1(get_file, capfd):
#     """
#     This test tests to see if the mutant enters branch 1
#     """
#     os.system(fr"{get_file}")
#     out, err = capfd.readouterr()
#     assert r"Entered first if statement" in out

# @pytest.mark.branch
# def test_branch_2(get_file, capfd):
#     """
#     This test tests to see if the mutant enters branch 2
#     """
#     os.system(fr"{get_file}")
#     out, err = capfd.readouterr()
#     assert r"Entered second if statement" in out

# @pytest.mark.branch
# def test_branch_3(get_file, capfd):
#     """
#     This test tests to see if the mutant enters branch 3
#     """
#     os.system(fr"{get_file}")
#     out, err = capfd.readouterr()
#     assert r"Entered third if statement" in out

# @pytest.mark.branch
# def test_branch_4(get_file, capfd):
#     """
#     This test tests to see if the mutant enters branch 4
#     """
#     os.system(fr"{get_file}")
#     out, err = capfd.readouterr()
#     assert r"Entered fourth if statement" in out


# @pytest.fixture()
# def get_file(request):
#     return request.config.getoption("--file")