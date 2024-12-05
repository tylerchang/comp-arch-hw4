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
        nums = []

        with open(fname, 'w') as file:
            for n in range(N):
                num = np.random.choice(self.space)
                nums.append(num)
                file.write(str(num) + '\n')

        plt.figure(figsize=(10, 6))
        plt.hist(nums, bins=30, density=True, alpha=0.6, color='blue', label='Sample Histogram')
        plt.title(f"Histogram of Uniform Samples")
        plt.xlabel("Value")
        plt.ylabel("Density")

        plt.show()





class Normal:
    def __init__(self, mean, variance):
        self.mean = mean
        self.var = variance
    
    def write_samples(self, N, fname):
        with open(fname, 'w') as file:
            count = 0
            saml = []
            while (count < 1000):
                samples = stats.norm.rvs(loc=self.mean, scale=self.var**0.5, size=N)
                for num in samples:
                    num_int = round(num)
                    if (num > 0):
                        file.write(str(num_int) + '\n')
                        count+=1
                        saml.append(num_int)
        
        plt.figure(figsize=(10, 6))
        plt.hist(saml, bins=30, density=True, alpha=0.6, color='blue', label='Sample Histogram')

        x = sorted(saml)
        pdf = stats.norm.pdf(x, loc=self.mean, scale=self.var**0.5)
        plt.plot(x, pdf, 'r-', lw=2, label='True Distribution')
        plt.title(f"Histogram and PDF of Normal Samples\n(Mean={self.mean}, Variance={self.var})")
        plt.xlabel("Value")
        plt.ylabel("Density")
        plt.legend()
        plt.grid(True)
        plt.show()


class BiModal:
    def __init__(self, mean1, mean2, var1, var2):
        self.mean1 = mean1
        self.var1 = var1
        self.mean2 = mean2
        self.var2 = var2
    
    def write_samples(self, N, fname):

        n1, p1 = 1000, 0.25  # n=500 trials, p=0.5, expected mean = 250

        # Second distribution with peak at 750
        n2, p2 = 1000, 0.75  # n=1000 trials, p=0.75, expected mean = 750

        # Generate samples
        samples1 = np.random.binomial(n1, p1, 500)  # Samples from first binomial
        samples2 = np.random.binomial(n2, p2, 500)  # Samples from second binomial

        # Combine the samples
        combined_samples = np.concatenate([samples1, samples2])

        # Plot the histogram of the combined samples
        plt.hist(combined_samples, bins=range(0, 1001, 1), density=True, alpha=0.7, color='b')
        plt.title("Combined Binomial Distribution with Two Peaks at 250 and 750")
        plt.xlabel("Value")
        plt.ylabel("Frequency")
        plt.show()

        with open(fname, 'w') as file:
            for num in combined_samples:
                file.write(str(num) + '\n')





class Uneven_binomial:
    def __init__(self, mean1, mean2, var1, var2):
        self.mean1 = mean1
        self.var1 = var1
        self.mean2 = mean2
        self.var2 = var2
    
    def write_samples(self, N, fname):

        count = 0
        saml = []
        
        # Ensure equal number of samples for each distribution
        while count < N:
            # Generate samples for the first distribution (mean1, var1)
            samples = stats.norm.rvs(loc=self.mean1, scale=self.var1**0.5, size=round(N/2))
            for num in samples:
                num_int = round(num)
                if num_int > 0:
                    count += 1
                    saml.append(num_int)
        
        count = 0
        while count < N:
            # Generate samples for the second distribution (mean2, var2)
            samples = stats.norm.rvs(loc=self.mean2, scale=self.var2**0.5, size=round(N/2))
            for num in samples:
                num_int = round(num)
                if num_int > 0:
                    count += 1
                    saml.append(num_int)
        
        # Plot the histogram
        plt.figure(figsize=(10, 6))
        plt.hist(saml, bins=30, density=True, alpha=0.6, color='blue', label='Sample Histogram')

        # Create the PDFs for both distributions
        x = sorted(saml)
        pdf1 = stats.norm.pdf(x, loc=self.mean1, scale=self.var1**0.5)
        pdf2 = stats.norm.pdf(x, loc=self.mean2, scale=self.var2**0.5)

        with open(fname, 'w') as file:
            for num in saml:
                file.write(str(num) + '\n')

        # Plot both PDFs
        # plt.plot(x, pdf1, 'g-', lw=2, label=f'True Distribution 1 (Mean={self.mean1}, Var={self.var1})')
        # plt.plot(x, pdf2, 'g-', lw=2, label=f'True Distribution 2 (Mean={self.mean2}, Var={self.var2})')

        plt.title(f"Histogram and PDF of Generated Samples with Long Tail\n(Mean1={self.mean1}, Var1={self.var1}), (Mean2={self.mean2}, Var2={self.var2})")
        plt.xlabel("Value")
        plt.ylabel("Density")
        plt.legend()
        plt.grid(True)
        plt.show()



if __name__ == '__main__':
    uni_dist = Uniform(0, 1000)
    uni_dist.write_samples(1000, 'uniform_1K.txt')

    uni_dist_1 = Normal(500, 27778)
    uni_dist_1.write_samples(1000, 'uniform_500.txt')

    uni_dist_2 = Normal(250, 27778)
    uni_dist_2.write_samples(1000, 'uniform_250.txt')
    
    tail_dist = Uneven_binomial(250, 750, 27778,27778)
    tail_dist.write_samples(1000, 'bimodal_longTail.txt')

       
    bi_dist = BiModal(250, 750, 27778,27778)
    bi_dist.write_samples(1000, 'bimodal_even.txt')
