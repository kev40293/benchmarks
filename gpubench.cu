#include <stdio.h>
#include <math.h>

__global__ void add(int* a, int * b, int *c) {
	int i = threadIdx.x;
	c[i] = a[i] + b[i];
}

__global__ void add_double(double *a, double *b, double*c) {
	int i = threadIdx.x;
	c[i] = a[i] + b[i];
}

void bench_gpu_memory() {
	clock_t start, stop;
	int *d_c, *d_cd; // device copies
	// Memory benchmarks
	for (int i=2; i < 3; i++) {
		int bsize = pow(1024, i);
		int wsize = 1024*1024*512;
		double rate;
		char * moo = (char*)malloc(bsize);
		cudaMalloc((void**) &d_c, 1024*1024);
		cudaMalloc((void**) &d_cd, 1024*1024);
		printf("%d\n", wsize);
/*
		start = clock();
		for (int k =0; k < wsize; k++) {
			cudaMemcpy(d_c, (char*) moo, bsize, cudaMemcpyHostToDevice);
		}
		stop = clock();
		rate = (wsize * bsize / (1024*1024)) / ((stop-start) / 1000000.0);
		printf("Host to device, bs =  %d, bandwidth = %Lf MB/s\n", bsize, rate);

		start = clock();
		for (int k =0; k < wsize; k++) {
			cudaMemcpy(d_c, (char*) moo, bsize, cudaMemcpyDeviceToHost);
		}
		stop = clock();
		rate = (wsize * bsize / (1024*1024)) / ((stop-start) / 1000000.0);
		printf("device to host, bs =  %d, bandwidth = %lf MB/s\n", bsize, rate);
*/

		start = clock();
		for (int k =0; k < wsize; k++) {
			cudaMemcpy(d_c, d_cd, bsize, cudaMemcpyDeviceToDevice);
		}
		stop = clock();
		printf("%d\n", stop-start);
		rate = (wsize * bsize / (1024)) / ((stop-start) / 1000000.0);
		printf("device to device, bs =  %d, bandwidth = %lf KB/s\n", bsize, rate);
		cudaFree(d_c);
		cudaFree(d_cd);
		free(moo);
	}

}

void bench_gpu_ops() {
	int prob_size = 1000;
	int a[prob_size],b[prob_size]; // Host copies
	double ad[prob_size], bd[prob_size];
	clock_t start, stop;
	int *d_a, *d_b, *d_c; // device copies
	double *d_ad, *d_bd, *d_cd;

	int size = sizeof(int) * prob_size;
	int sized = sizeof(double) * prob_size;

	// Allocate space
	cudaMalloc((void **) &d_a, size);
	cudaMalloc((void **) &d_b, size);
	cudaMalloc((void **) &d_c, size);
	for (int i = 0; i < prob_size; i++) {
		a[i] = i;
		ad[i] = i;
		b[i] = prob_size - i;
		bd[i] = prob_size - i;
	}
	// Copy inputs to dev
	cudaMemcpy(d_a, &a, size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_b, &b, size, cudaMemcpyHostToDevice);
	start = clock();
	for (int i = 0; i < 1000000; i++) {
		add<<<1,1>>>(d_a, d_b, d_c);
	}
	stop = clock();
	int iop_time = stop-start;
	cudaFree(d_a);
	cudaFree(d_b);

	cudaMalloc((void **) &d_ad, sized);
	cudaMalloc((void **) &d_bd, sized);
	cudaMalloc((void **) &d_cd, sized);
	printf("Time taken (integer) = %d ms\n", iop_time);
	printf("GPU IOPs = %f\n", 1000000.0*prob_size / (iop_time/1000000.0));

	cudaMemcpy(d_ad, &ad, sized, cudaMemcpyHostToDevice);
	cudaMemcpy(d_bd, &bd, sized, cudaMemcpyHostToDevice);
	start = clock();
	for (int i = 0; i < 1000000; i++) {
		add_double<<<1,1>>>(d_ad, d_bd, d_cd);
	}
	stop = clock();
	int flop_time = stop-start;
	//printf("Our result is %d\n", c[0]);
	cudaFree(d_ad);
	cudaFree(d_bd);
	cudaFree(d_cd);
	printf("Time taken (double) = %d ms\n", flop_time);
	printf("GPU FLOPs = %f\n", 1000000.0*prob_size / (flop_time/1000000.0));
}

int main(int argc, char** argv) {
	if (argc > 1) {
		switch (argv[1][0]) {
			case 'g':
				bench_gpu_ops(); break;
			case 'm':
				bench_gpu_memory(); break;
		}
	}

	return 0;
}
