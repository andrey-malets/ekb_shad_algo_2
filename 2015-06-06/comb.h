#include <cassert>
#include <vector>

template<typename I, typename T> class Comb {
 public:
  void reserve(size_t rows, size_t size) {
    index_.reserve(rows);
    data_.reserve(size);
  }

  void end_row() {
    assert(data_.size() < (1ll << (8*sizeof(I))));
    index_.push_back(data_.size());
  }

  void append(const T& value) {
    data_.push_back(value);
  }

  size_t rows() const {
    return index_.size();
  }

  size_t length(size_t row) const {
    return index_[row] - get_start(row);
  }

  const T& at(size_t row, size_t col) const {
    return data_[get_start(row) + col];
  }

 private:
  size_t get_start(size_t row) const {
    assert(row < index_.size());
    return (row == 0) ? 0 : index_[row - 1];
  }

  std::vector<I> index_;
  std::vector<T> data_;
};
