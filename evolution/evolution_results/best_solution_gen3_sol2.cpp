vector<char> isPrime(maxLen + 1, true);
vector<int>  primes;
isPrime[0] = isPrime[1] = false;
for (int i = 2; i <= maxLen; ++i) {
    if (isPrime[i]) primes.push_back(i);
    for (int p : primes) {
        long long v = 1LL * i * p;
        if (v > maxLen) break;
        isPrime[v] = false;
        if (i % p == 0) break;
    }
}
