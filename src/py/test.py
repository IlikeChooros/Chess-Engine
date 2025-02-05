import multiprocessing

# one dimension of the 2d array which is shared
DIM = 5000

import numpy as np
from multiprocessing import shared_memory, Process, Lock
from multiprocessing import cpu_count, current_process
import time


def add_one_v1(shr_name, lock):
    existing_shm = shared_memory.SharedMemory(name=shr_name)
    np_array = np.ndarray((DIM, DIM,), dtype=np.int64, buffer=existing_shm.buf)
    print('before one', np_array.sum())
    lock.acquire()
    np_array[:] = np_array[:] + 1
    lock.release()
    time.sleep(1)
    print('after one', np_array.sum())
    existing_shm.close()


def add_one_v2(shr_name):
    existing_shm = shared_memory.SharedMemory(name=shr_name)
    np_array = np.ndarray((DIM, DIM,), dtype=np.int64, buffer=existing_shm.buf)
    print('before one', np_array.sum())
    lock.acquire()
    np_array[:] = np_array[:] + 1
    lock.release()
    time.sleep(1)
    print('after one', np_array.sum())
    existing_shm.close()


def create_shared_block(a):
    shm = shared_memory.SharedMemory(create=True, size=a.nbytes)
    # # Now create a NumPy array backed by shared memory
    np_array = np.ndarray(a.shape, dtype=np.int64, buffer=shm.buf)
    np_array[:] = a[:]  # Copy the original data into shared memory
    return shm, np_array


def example_with_Process():
    lock = Lock()
    matrix = np.ones(shape=(DIM, DIM), dtype=np.int64)  # Start with an existing NumPy array
    print("creating shared block")
    shr, np_array = create_shared_block(matrix)

    print('Num of CPUs: {}'.format(cpu_count()))
    processes = []
    for i in range(cpu_count()):
        _process = Process(target=add_one_v1, args=(shr.name, lock))
        processes.append(_process)
        _process.start()

    for _process in processes:
        _process.join()

    print("Final array (expected values: {})".format(cpu_count() + 1))
    print(np_array[:10])
    print(np_array[10:])

    shr.close()
    shr.unlink()


def init_pool_processes(the_lock):
    '''Initialize each process with a global variable lock.
    '''
    global lock
    lock = the_lock


def example_with_Pool():
    lock = Lock()
    matrix = np.ones(shape=(DIM, DIM), dtype=np.int64)  # Start with an existing NumPy array
    print("creating shared block")
    shr, np_array = create_shared_block(matrix)

    print('Num of CPUs: {}'.format(cpu_count()))
    pool = multiprocessing.Pool(
        processes=cpu_count(),
        initializer=init_pool_processes,
        initargs=(lock,)
    )
    data_array = []
    for i in range(50):
        data_array.append(shr.name)
    pool.map(add_one_v2, data_array)

    print("Final array (expected values: {})".format(50 + 1))
    print(np_array[:10])
    print(np_array[10:])

    shr.close()
    shr.unlink()


if __name__ == '__main__':
    example_with_Process()
    example_with_Pool()
