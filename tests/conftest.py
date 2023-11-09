import os
import pathlib
import subprocess

import pytest


def run_simjob(output_directory: pathlib.Path):
    subprocess.run(
        ['simjob'],
        cwd = output_directory,
        check = True,
        capture_output = True
    )

@pytest.fixture()
def simjob_filepath(request):
    """Retreive the simjob.slcio example file from the test cache

    This example file is only regenerated if the cache has been cleared
    using the `--clear-cache` option when running pytest.
    """

    path = request.config.cache.get('test/file', None)
    if path is None:
        parent_dir = request.config.cache.mkdir('test_file')
        run_simjob(parent_dir)
        path = str(parent_dir / 'simjob.slcio')
        request.config.cache.set('test/file', path)
    return path
