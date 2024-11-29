# Generate samples from a particular probability distribution
import numpy as np
from scipy import stats


class Uniform:
    def __init__(self, min, max):
        self.min = min
        self.max = max
        self.space = np.arange(min, max+1)
    
    def write_samples(self, N, fname):
        with open(fname, 'w') as file:
            for n in range(N):
                num = np.random.choice(self.space)
                file.write(str(num) + '\n')


if __name__ == '__main__':
    dist = Uniform(0, 10)
    dist.write_samples(50, 'uniform10.txt')
