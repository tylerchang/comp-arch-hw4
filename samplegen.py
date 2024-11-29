# Generate samples from a particular probability distribution
import matplotlib.pyplot as plt
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


class Normal:
    def __init__(self, mean, variance):
        self.mean = mean
        self.var = variance
    
    def write_samples(self, N, fname):
        with open(fname, 'w') as file:
            samples = stats.norm.rvs(loc=self.mean, scale=self.var**0.5, size=N)
            for num in samples:
                file.write(str(num) + '\n')
        
        # plt.figure(figsize=(10, 6))
        # plt.hist(samples, bins=30, density=True, alpha=0.6, color='blue', label='Sample Histogram')

        # x = sorted(samples)
        # pdf = stats.norm.pdf(x, loc=self.mean, scale=self.var**0.5)
        # plt.plot(x, pdf, 'r-', lw=2, label='True Distribution')

        # plt.title(f"Histogram and PDF of Generated Samples\n(Mean={self.mean}, Variance={self.var})")
        # plt.xlabel("Value")
        # plt.ylabel("Density")
        # plt.legend()
        # plt.grid(True)
        # plt.show()


if __name__ == '__main__':
    uni_dist = Uniform(0, 10)
    uni_dist.write_samples(10000, 'uniform10.txt')
    norm_dist = Normal(500, 50)
    norm_dist.write_samples(10000, 'norm500')
