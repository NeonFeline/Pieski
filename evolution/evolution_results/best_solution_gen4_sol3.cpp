vector<char> isPrime(N + 1, true);
isPrime[0] = isPrime[1] = false;
vector<int> primes;
for (int i = 2; i <= N; ++i) {
    if (isPrime[i]) primes.push_back(i);
    for (int p : primes) {
        long long v = 1LL * i * p;
        if (v > N) break;
        isPrime[v] = false;
        if (i % p == 0) break;
    }
}
