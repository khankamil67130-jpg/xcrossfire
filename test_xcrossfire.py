import unittest
import xcrossfire
from array import import array
import random
import time

class TestXCrossfireVectorRadar(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.total_elements = 10000000
        cls.raw_list = [random.randint(10, 5000000) for _ in range(cls.total_elements)]
        cls.test_array = array('i', cls.raw_list)

    def test_case_01_target_present_verification(self):
        known_index = 7543210
        known_target = -7777
        self.raw_list[known_index] = known_target
        sync_array = array('i', self.raw_list)

        start = time.perf_counter_ns()
        result_index = xcrossfire.search(sync_array, known_target)
        end = time.perf_counter_ns()