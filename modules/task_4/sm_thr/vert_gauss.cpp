// Copyright 2021 Molotkova Svetlana
#include <vector>
#include <random>
#include <ctime>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include "../../../modules/task_4/sm_thr/vert_gauss.h"
#include "../../../3rdparty/unapproved/unapproved.h"

int clamp(int n, int upper, int lower) {
  return n <= lower ? lower : n >= upper ? upper : n;
}

std::vector<double> matrixCreation(int rows, int cols) {
  if ((rows <= 0) || (cols <= 0))
     throw std::invalid_argument("Matrix can't be created");
  std::mt19937 gen;
  gen.seed(static_cast<unsigned int>(time(0)));
  std::vector<double> result(rows * cols);
  for (int i = 0; i < rows * cols; i++)
    result[i] = gen() % 256;
  return result;
}

std::vector<double> transpose(const std::vector<double>& matrix, int rows, int cols) {
  std::vector<double> result(rows * cols);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      result[i + j * rows] = matrix[i * cols + j];
    }
  }
  return result;
}

std::vector<double> gaussKernel(int radius, int sigma) {
  // determine the size of the kernel = 3б, radius = 1
  const int size = 2 * radius + 1;
  // coef of normalization of kernel
  double norm = 0;
  // creating gauss kernel
  std::vector<double> kernel(size * size);
  // calculating linear filter kernel
  for (int i = -radius; i <= radius; i++) {
    for (int j = -radius; j <= radius; j++) {
      int idx = (i + radius) * size + j + radius;
      kernel[idx] = exp(-(i * i + j * j) / (sigma * sigma));
      norm += kernel[idx];
    }
  }

  // normalizing the kernel
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
        kernel[i * size + j] /= norm;
  return kernel;
}

std::vector<double> gaussFilter(const std::vector<double>& matrix, int rows, int cols,
  int radius, int sigma) {
  std::vector<double> resultMatrix(rows * cols);
  const unsigned int size = 2 * radius + 1;
  std::vector<double> kernel = gaussKernel(radius, sigma);
  for (int x = 0; x < rows; x++) {
    for (int y = 0; y < cols; y++) {
      double res = 0;
      for (int i = -radius; i <= radius; i++) {
        for (int j = -radius; j <= radius; j++) {
          int idx = (i + radius) * size + j + radius;
          int x_ = clamp(x + j, rows - 1, 0);
          int y_ = clamp(y + i, cols - 1, 0);
          double value = matrix[x_ * cols + y_];
          res += value * kernel[idx];
        }
      }
      res = clamp(res, 255, 0);
      resultMatrix[x * cols + y] = res;
    }
  }
  return resultMatrix;
}

std::vector<double> gaussFilter_par(const std::vector<double>& matrix, int rows, int cols,
  int radius, int sigma) {
  std::vector<double> resultMatrix(rows * cols);
  const unsigned int size = 2 * radius + 1;
  std::vector<double> kernel = gaussKernel(radius, sigma);
  const int nthreads = std::thread::hardware_concurrency();
  std::thread* threads = new std::thread[nthreads];
  int offset = rows / nthreads;
  int rem = rows % nthreads;
  for (int i = 0; i < nthreads; i++) {
        threads[i] = std::thread([offset, nthreads, rem, cols, radius, size, rows,
        &kernel, &matrix, &resultMatrix](int thread) {
          int begin = thread * offset;
          int numrows;
          if (thread == nthreads - 1) {
              numrows = offset + rem;
          } else {
              numrows = offset;
          }
            for (int x = begin; x < begin + numrows; x++) {
                for (int y = 0; y < cols; y++) {
                    double res = 0.0;
                    for (int k = -radius; k <= radius; k++) {
                    for (int j = -radius; j <= radius; j++) {
                    int idx = (k + radius) * size + j + radius;
                    int x_ = clamp(x + j, rows - 1, 0);
                    int y_ = clamp(y + k, cols - 1, 0);
                    double value = matrix[x_ * cols + y_];
                    res += value * kernel[idx];
        }
      }
      res = clamp(res, 255, 0);
      resultMatrix[x * cols + y] = res;
                }
            }
        }, i);
    }
    for (int i = 0; i < nthreads; i++) {
        threads[i].join();
    }

    delete[] threads;
  return resultMatrix;
  }

