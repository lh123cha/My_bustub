//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// p0_starter.h
//
// Identification: src/include/primer/p0_starter.h
//
// Copyright (c) 2015-2020, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include "common/exception.h"

#include "common/logger.h"

namespace bustub {

/**
 * The Matrix type defines a common
 * interface for matrix operations.
 */
template <typename T>
class Matrix {
 protected:
  /**
   * TODO(P0): Add implementation
   *
   * Construct a new Matrix instance.
   * @param rows The number of rows
   * @param cols The number of columns
   *
   */
  Matrix(int rows, int cols) : rows_(rows), cols_(cols), linear_(nullptr) {
    if (rows_ <= 0 || cols_ <= 0) {
      rows_ = cols_ = -1;
    } else {
      linear_ = new T[rows_ * cols_];
    }
  }


  /** The number of rows in the matrix */
  int rows_;
  /** The number of columns in the matrix */
  int cols_;


  /**
   * TODO(P0): Allocate the array in the constructor.
   * TODO(P0): Deallocate the array in the destructor.
   * A flattened array containing the elements of the matrix.
   */
  T *linear_;

 public:
  /** @return The number of rows in the matrix */
  virtual int GetRowCount() const = 0;

  /** @return The number of columns in the matrix */
  virtual int GetColumnCount() const = 0;

  /**
   * Get the (i,j)th matrix element.
   *
   * Throw OUT_OF_RANGE if either index is out of range.
   *
   * @param i The row index
   * @param j The column index
   * @return The (i,j)th matrix element
   * @throws OUT_OF_RANGE if either index is out of range
   */
  virtual T GetElement(int i, int j) const = 0;

  /**
   * Set the (i,j)th matrix element.
   *
   * Throw OUT_OF_RANGE if either index is out of range.
   *
   * @param i The row index
   * @param j The column index
   * @param val The value to insert
   * @throws OUT_OF_RANGE if either index is out of range
   */
  virtual void SetElement(int i, int j, T val) = 0;

  /**
   * Fill the elements of the matrix from `source`.
   *
   * Throw OUT_OF_RANGE in the event that `source`
   * does not contain the required number of elements.
   *
   * @param source The source container
   * @throws OUT_OF_RANGE if `source` is incorrect size
   */
  virtual void FillFrom(const std::vector<T> &source) = 0;

  /**
   * Destroy a matrix instance.
   * TODO(P0): Add implementation
   */
  virtual ~Matrix(){
    delete[] linear_;
  }
};

/**
 * The RowMatrix type is a concrete matrix implementation.
 * It implements the interface defined by the Matrix type.
 */
template <typename T>
class RowMatrix : public Matrix<T> {
 public:
  /**
   * TODO(P0): Add implementation
   *
   * Construct a new RowMatrix instance.
   * @param rows The number of rows
   * @param cols The number of columns
   */
  RowMatrix(int rows, int cols) : Matrix<T>(rows, cols) {
    data_ = new T *[this->rows_ * this->cols_];
    for (int row = 0; row < this->rows_; row++) {
      data_[row] = &this->linear_[row * this->cols_];
    }
  }

  /**
   * TODO(P0): Add implementation
   * @return The number of rows in the matrix
   */
  int GetRowCount() const override {
    return this->rows_;
  }

  /**
   * TODO(P0): Add implementation
   * @return The number of columns in the matrix
   */
  int GetColumnCount() const override { return this->cols_; }

  /**
   * TODO(P0): Add implementation
   *
   * Get the (i,j)th matrix element.
   *
   * Throw OUT_OF_RANGE if either index is out of range.
   *
   * @param i The row index
   * @param j The column index
   * @return The (i,j)th matrix element
   * @throws OUT_OF_RANGE if either index is out of range
   */
  T GetElement(int i, int j) const override {
//    throw NotImplementedException{"RowMatrix::GetElement() not implemented."};
    if(i<0||i>=GetRowCount()||j<0||j>= GetColumnCount()){
      throw Exception(ExceptionType::OUT_OF_RANGE,"RowMatrix()::GetElement out of range");
    }
    return data_[i][j];
  }

  /**
   * Set the (i,j)th matrix element.
   *
   * Throw OUT_OF_RANGE if either index is out of range.
   *
   * @param i The row index
   * @param j The column index
   * @param val The value to insert
   * @throws OUT_OF_RANGE if either index is out of range
   */
  void SetElement(int i, int j, T val) override {
    if(i<0||i>= GetRowCount()||j<0||j>= GetColumnCount()){
      throw Exception(ExceptionType::OUT_OF_RANGE,"RowMatrix()::SetElement out of range");
    }
    data_[i][j]=val;
  }

  /**
   * TODO(P0): Add implementation
   *
   * Fill the elements of the matrix from `source`.
   *
   * Throw OUT_OF_RANGE in the event that `source`
   * does not contain the required number of elements.
   *
   * @param source The source container
   * @throws OUT_OF_RANGE if `source` is incorrect size
   */
  void FillFrom(const std::vector<T> &source) override {
    int rows=GetRowCount();
    int cols = GetColumnCount();
    LOG_INFO("source size is %d",(int)source.size());
    LOG_INFO("row num is %d",rows);
    LOG_INFO("col num is %d",cols);
    if(static_cast<int>(source.size())==rows*cols){
      for(int i=0;i<rows;i++){
        for(int j=0;j< cols;j++){
          data_[i][j]=source[i* this->cols_+j];
        }
      }
    }else{
        throw Exception(ExceptionType::OUT_OF_RANGE,"RowMatrix()::FillFrom out of range");
    }
  }

  /**
   * TODO(P0): Add implementation
   *
   * Destroy a RowMatrix instance.
   */
  ~RowMatrix() override {
      delete[] data_;
  };

 private:
  /**
   * A 2D array containing the elements of the matrix in row-major format.
   *
   * TODO(P0):
   * - Allocate the array of row pointers in the constructor.
   * - Use these pointers to point to corresponding elements of the `linear` array.
   * - Don't forget to deallocate the array in the destructor.
   */
  T **data_;
};

/**
 * The RowMatrixOperations class defines operations
 * that may be performed on instances of `RowMatrix`.
 */
template <typename T>
class RowMatrixOperations {
 public:
  /**
   * Compute (`matrixA` + `matrixB`) and return the result.
   * Return `nullptr` if dimensions mismatch for input matrices.
   * @param matrixA Input matrix
   * @param matrixB Input matrix
   * @return The result of matrix addition
   */
  static std::unique_ptr<RowMatrix<T>> Add(const RowMatrix<T> *matrixA,const RowMatrix<T> *matrixB) {
    // TODO(P0): Add implementation
    int Arows=matrixA->GetRowCount();
    int Acols= matrixA->GetColumnCount();
    int Brows=matrixB->GetRowCount();
    int Bcols = matrixB->GetColumnCount();
    RowMatrix<T>* r = new RowMatrix<T>(Arows,Bcols);
    if(Arows!=Brows||Acols!=Bcols) {
      return std::unique_ptr<RowMatrix<T>>(nullptr);
    }
    for(int i=0;i<Arows;i++){
      for(int j=0;j<Acols;j++){
        r->SetElement(i,j,matrixA->GetElement(i,j)+matrixB->GetElement(i,j));
      }
    }
    return std::unique_ptr<RowMatrix<T>>(r);
  }

  /**
   * Compute the matrix multiplication (`matrixA` * `matrixB` and return the result.
   * Return `nullptr` if dimensions mismatch for input matrices.
   * @param matrixA Input matrix
   * @param matrixB Input matrix
   * @return The result of matrix multiplication
   */
  static std::unique_ptr<RowMatrix<T>> Multiply(const RowMatrix<T> *matrixA, const RowMatrix<T> *matrixB) {
    // TODO(P0): Add implementation
    int Arows=matrixA->GetRowCount();
    int Acols= matrixA->GetColumnCount();
    int Brows=matrixB->GetRowCount();
    int Bcols = matrixB->GetColumnCount();
    if(Acols!=Brows){
      return std::unique_ptr<RowMatrix<T>>(nullptr);
    }
    RowMatrix<T> *result = new RowMatrix<T>(Arows,Bcols);
    for(int i=0;i<Arows;i++){
      for(int j=0;j<Bcols;j++){
        int temp=0;
        for(int k=0;k<Acols;k++){
          temp+=matrixA->GetElement(i,k)*matrixB->GetElement(k,j);
        }
        result->SetElement(i,j,temp);
      }
    }
    return std::unique_ptr<RowMatrix<T>>(result);
  }

  /**
   * Simplified General Matrix Multiply operation. Compute (`matrixA` * `matrixB` + `matrixC`).
   * Return `nullptr` if dimensions mismatch for input matrices.
   * @param matrixA Input matrix
   * @param matrixB Input matrix
   * @param matrixC Input matrix
   * @return The result of general matrix multiply
   */
  static std::unique_ptr<RowMatrix<T>> GEMM(const RowMatrix<T> *matrixA, const RowMatrix<T> *matrixB,
                                            const RowMatrix<T> *matrixC) {
    // TODO(P0): Add implementation
    return Add(Multiply(matrixA,matrixB),matrixC);
  }
};
}  // namespace bustub
