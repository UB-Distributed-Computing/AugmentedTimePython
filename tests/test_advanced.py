# -*- coding: utf-8 -*-

from .context import AugmentedTime

import unittest


class AdvancedTestSuite(unittest.TestCase):
    """Advanced test cases."""

    def test_thoughts(self):
        AugmentedTime.get_timestamp()


if __name__ == '__main__':
    unittest.main()
