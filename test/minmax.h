#include <cstdlib>
#include <cstring>
#include <time.h>
#include <algorithm>
#include <functional>
#include <vector>
#ifdef _MSC_VER
# include <random>
#else
# include <tr1/random>
#endif
#include <boost/cstdint.hpp>

#include <cxxtest/TestSuite.h>

#include "Controller/Controller.h"
#include "Quantize.h"

#include "util-test.h"

#define PREFIX "/home/tfogal/data"

template <typename T>
struct testfile {
  const char *file;
  size_t bytes_to_skip;
  T data_min;
  T data_max;
  typedef T base_type;
};

template <typename T>
inline void check_equality(T a, T b) { TS_ASSERT_EQUALS(a, b); }

template <>
inline void check_equality<double>(double a, double b) {
  TS_ASSERT_DELTA(a,b, 0.0001);
}

template <typename T>
struct test_quant : public std::unary_function<testfile<T>, void> {
  void operator()(const testfile<T> &tf) const {
#ifdef VERBOSE
    {
      std::ostringstream trace;
      trace << "testing " << sizeof(T)*8 << "bit data in " << tf.file;
      TS_TRACE(trace.str());
    }
#endif

    std::string fn = std::string(tf.file);
    const size_t sz = filesize(fn.c_str());
    const size_t n_elems = sz / sizeof(T);

    std::vector<UINT64> hist;
    {
#ifdef VERBOSE
      TS_TRACE("raw_data_src");
#endif
      Unsigned12BitHistogram<T> histw(hist);
      LargeRAWFile raw(fn);
      raw.Open(false);
      std::pair<T,T> mm = io_minmax<T>(raw_data_src<T>(raw), histw,
                                       TuvokProgress<UINT64>(n_elems));
      check_equality(tf.data_min, mm.first);
      check_equality(tf.data_max, mm.second);
      raw.Close();
    }
    {
#ifdef VERBOSE
      TS_TRACE("ios_data_src");
#endif
      Unsigned12BitHistogram<T> histw(hist);
      std::ifstream fs(fn.c_str());
      std::pair<T,T> mm = io_minmax<T>(ios_data_src<T>(fs), histw,
                                       TuvokProgress<UINT64>(n_elems));
      check_equality(tf.data_min, mm.first);
      check_equality(tf.data_max, mm.second);
      fs.close();
    }
  }
};

// Create a temporary file and return the name.
// This isn't great -- there's a race between when we close and reopen it --
// but there's no (standard) way to turn a file descriptor into a std::fstream.
static std::string tmpfile(std::ofstream& ofs, std::ios_base::openmode mode)
{
  char templ[64];
  strcpy(templ, "iotest.XXXXXX");
  int fd = mkstemp(templ);
  close(fd);
  ofs.open(templ, mode);
  return std::string(templ);
}

// Data generation code.
namespace {
  // Generates data with a constant value
  template <typename T>
  void gen_constant(std::ostream& os, const size_t sz, const T& val) {
    for(size_t i=0; i < sz/sizeof(T); ++i) {
      os.write(reinterpret_cast<const char*>(&val), sizeof(T));
    }
  }

  // Generates data along a normal distribution with the given mean and
  // standard deviation.
  template <typename T>
  std::pair<T,T> gen_normal(std::ostream& os, const size_t sz,
                            const T& mean, const T& stddev) {
    std::pair<T,T> minmax = std::make_pair(
      std::numeric_limits<T>::max(),
      -(std::numeric_limits<T>::max()-1) // bleh, not great.
    );
    // double: tr1 RNGs are only defined for FP types.  We'll generate double
    // and just cast to T.
    std::tr1::variate_generator<std::tr1::mt19937,
                                std::tr1::normal_distribution<double> > vg(
      std::tr1::mt19937(time(NULL)),
      std::tr1::normal_distribution<double>(mean, stddev)
    );
    for(size_t i=0; i < sz/sizeof(T); ++i) {
      T v = static_cast<T>(vg());
      minmax.first = std::min(minmax.first, v);
      minmax.second = std::max(minmax.second, v);
      os.write(reinterpret_cast<const char*>(&v), sizeof(T));
    }
    return minmax;
  }
}

namespace {
  template<typename T>
  void t(size_t sz, T mean, T stddev) {
    std::ofstream dataf;
    std::string fn = tmpfile(dataf, std::ios::out | std::ios::binary);
    std::pair<T,T> minmax = gen_normal<T>(dataf, sz, mean, stddev);
    dataf.close();
    {
      struct testfile<T> tf = { fn.c_str(), 0, minmax.first, minmax.second };
      test_quant<T> tester;
      tester(tf);
    }
    remove(fn.c_str());
  }
  template <typename T>
  void t_constant(size_t sz, T value) {
    std::ofstream dataf;
    std::string fn = tmpfile(dataf, std::ios::out | std::ios::binary);
    gen_constant<T>(dataf, sz, value);
    dataf.close();
    {
      struct testfile<T> tf = { fn.c_str(), 0, value, value };
      test_quant<T> tester;
      tester(tf);
    }
    remove(fn.c_str());
  }
}

class MinMaxTesting : public CxxTest::TestSuite {
  public:
    void atest_short() {
      struct testfile<short> files_short[] = {
        {"data/short",0, -32765, 32741}
      };
      for_each(files_short, test_quant<short>());
    }

    void atest_ubyte() {
      struct testfile<unsigned char> files_ubyte[] = {
        {"data/ubyte",0, 0,255}
      };
      for_each(files_ubyte, test_quant<unsigned char>());
    }

    void atest_float() {
      struct testfile<float> files_float[] = {
        {"data/float",0, 1.3389827,235.573898},
      };
      for_each(files_float, test_quant<float>());
    }

    void atest_double() {
      struct testfile<double> files_double[] = {
        {"data/double",0, 1.3389827013, 235.5738983154},
      };
      for_each(files_double, test_quant<double>());
    }

    // We have a wide set of variables to test:
    //   every type: byte,ubyte, short,ushort, int,uint, INT64,UINT64,
    //               float, double
    //   fits in 12 bits, doesn't fit in 12 bits
    //   all values negative, spans 0, all positive
    //      pathological cases: all the same neg/pos value, all 0.
    //   file < in core size, file == in core size, file > in core size

    typedef signed char byte;
    // byte, (they always fit in 12 bits :), all neg, small file
    void test_byte_neg_lt_incore() { t<byte>(INCORESIZE/64, -90, 2); }
    // byte, (always fits in 12 bits ;), all neg, == INCORESIZE.
    void test_byte_neg_eq_incore() { t<byte>(INCORESIZE,    -90, 2); }
    // byte, (always fit in 12 bits ;), all neg, > INCORESIZE.
    void test_byte_neg_gt_incore() { t<byte>(INCORESIZE*2,  -90, 2); }
    // byte, (always fits in 12 bits ;), spans 0, < INCORESIZE
    void test_byte_span_lt_incore() { t<byte>(INCORESIZE/64,  0, 3); }
    // byte, (always fits in 12 bits ;), spans 0, == INCORESIZE
    void test_byte_span_eq_incore() { t<byte>(INCORESIZE,     0, 3); }
    // byte, (always fits in 12 bits ;), spans 0, > INCORESIZE
    void test_byte_span_gt_incore() { t<byte>(INCORESIZE*2,   0, 3); }

    void test_char_neg_lt_incore() { t<char>(INCORESIZE/64,  -90, 2); }
    void test_char_neg_eq_incore() { t<char>(INCORESIZE,     -90, 2); }
    void test_char_neg_gt_incore() { t<char>(INCORESIZE*2,   -90, 2); }
    void test_char_span_lt_incore() { t<char>(INCORESIZE/64,   0, 3); }
    void test_char_span_eq_incore() { t<char>(INCORESIZE,      0, 3); }
    void test_char_span_gt_incore() { t<char>(INCORESIZE*2,    0, 3); }
    void test_char_pos_lt_incore() { t<char>(INCORESIZE/64,   90, 4); }
    void test_char_pos_eq_incore() { t<char>(INCORESIZE,      90, 4); }
    void test_char_pos_gt_incore() { t<char>(INCORESIZE*2,    90, 4); }

    typedef unsigned char ubyte;
    void test_ubyte_neg_lt_incore() { t<ubyte>(INCORESIZE/64,  -90, 2); }
    void test_ubyte_neg_eq_incore() { t<ubyte>(INCORESIZE,     -90, 2); }
    void test_ubyte_neg_gt_incore() { t<ubyte>(INCORESIZE*2,   -90, 2); }
    void test_ubyte_span_lt_incore() { t<ubyte>(INCORESIZE/64,   0, 3); }
    void test_ubyte_span_eq_incore() { t<ubyte>(INCORESIZE,      0, 3); }
    void test_ubyte_span_gt_incore() { t<ubyte>(INCORESIZE*2,    0, 3); }
    void test_ubyte_pos_lt_incore() { t<ubyte>(INCORESIZE/64,   90, 4); }
    void test_ubyte_pos_eq_incore() { t<ubyte>(INCORESIZE,      90, 4); }
    void test_ubyte_pos_gt_incore() { t<ubyte>(INCORESIZE*2,    90, 4); }

    void test_short_neg_12bit_lt_incore() {
      t<short>(INCORESIZE/64, -4096, 32);
    }
    void test_short_neg_12bit_eq_incore() {
      t<short>(INCORESIZE, -4096, 32);
    }
    void test_short_neg_12bit_gt_incore() {
      t<short>(INCORESIZE*2, -4096, 32);
    }
    // negative, doesn't fit in 12 bits: center at -16384, w/ a std dev of
    // 4096.  Since the data will be normally distributed, 99.7% of the data
    // will be w/in 3 std devs, i.e. 99.7% of the data will be < -16384+3*4096
    // == -4096.  I guess there's a 0.3% chance of an outlier, but not only
    // would it have to be an outlier, it would have to be an outlier that's 4
    // std deviations away from the mean.
    void test_short_neg_not12bit_lt_incore() {
      t<short>(INCORESIZE/64, -16384, 4096);
    }
    void test_short_neg_not12bit_eq_incore() {
      t<short>(INCORESIZE, -16384, 4096);
    }
    void test_short_neg_not12bit_gt_incore() {
      t<short>(INCORESIZE*2, -16384, 4096);
    }
    void test_short_span_12bit_lt_incore() {
      t<short>(INCORESIZE/64, 0, 32);
    }
    void test_short_span_12bit_eq_incore() {
      t<short>(INCORESIZE, 0, 32);
    }
    void test_short_span_12bit_gt_incore() {
      t<short>(INCORESIZE*2, 0, 32);
    }
    void test_short_span_not12bit_lt_incore() {
      t<short>(INCORESIZE/64, 0, 8192);
    }
    void test_short_span_not12bit_eq_incore() {
      t<short>(INCORESIZE, 0, 8192);
    }
    void test_short_span_not12bit_gt_incore() {
      t<short>(INCORESIZE*2, 0, 8192);
    }
    void test_short_pos_12bit_lt_incore() {
      t<short>(INCORESIZE/64, 16384, 32);
    }
    void test_short_pos_12bit_eq_incore() {
      t<short>(INCORESIZE, 16384, 32);
    }
    void test_short_pos_12bit_gt_incore() {
      t<short>(INCORESIZE*2, 16384, 32);
    }
    void test_short_pos_not12bit_lt_incore() {
      t<short>(INCORESIZE/64, 16384, 4096);
    }
    void test_short_pos_not12bit_eq_incore() {
      t<short>(INCORESIZE, 16384, 4096);
    }
    void test_short_pos_not12bit_gt_incore() {
      t<short>(INCORESIZE*2, 16384, 4096);
    }

    typedef unsigned short ushort;
    void test_ushort_0_12b_lti() { t<ushort>(INCORESIZE/64, 0, 64); }
    void test_ushort_0_12b_ei()  { t<ushort>(INCORESIZE,    0, 64); }
    void test_ushort_0_12b_gti() { t<ushort>(INCORESIZE*2,  0, 64); }
    void test_ushort_0_n12b_lti() { t<ushort>(INCORESIZE/64, 0, 8192); }
    void test_ushort_0_n12b_ei()  { t<ushort>(INCORESIZE,    0, 16384); }
    void test_ushort_0_n12b_gti() { t<ushort>(INCORESIZE*2,  0, 32768); }
    void test_ushort_p_12b_lti() { t<ushort>(INCORESIZE/64, 30123, 64); }
    void test_ushort_p_12b_ei()  { t<ushort>(INCORESIZE,    30456, 64); }
    void test_ushort_p_12b_gti() { t<ushort>(INCORESIZE*2,  30789, 64); }
    void test_ushort_p_n12b_lti() { t<ushort>(INCORESIZE/64, 29487, 8192); }
    void test_ushort_p_n12b_ei()  { t<ushort>(INCORESIZE,    24891, 4096); }
    void test_ushort_p_n12b_gti() { t<ushort>(INCORESIZE*2,  23489, 2048); }

    void test_int_n_12b_lti() { t<int>(INCORESIZE/64, -65534, 32); }
    void test_int_n_12b_ei()  { t<int>(INCORESIZE,    -65534, 32); }
    void test_int_n_12b_gti() { t<int>(INCORESIZE*2,  -65534, 32); }
    void test_int_n_n12b_lti() { t<int>(INCORESIZE/64, -268435456, 4096); }
    void test_int_n_n12b_ei()  { t<int>(INCORESIZE,    -268435456, 4096); }
    void test_int_n_n12b_gti() { t<int>(INCORESIZE*2,  -268435456, 4096); }
    void test_int_0_12b_lti() { t<int>(INCORESIZE/64, 0, 128); }
    void test_int_0_12b_ei()  { t<int>(INCORESIZE,    0, 128); }
    void test_int_0_12b_gti() { t<int>(INCORESIZE*2,  0, 128); }
    void test_int_0_n12b_lti() { t<int>(INCORESIZE/64, 0, 4096); }
    void test_int_0_n12b_ei()  { t<int>(INCORESIZE,    0, 4096); }
    void test_int_0_n12b_gti() { t<int>(INCORESIZE*2,  0, 4096); }
    void test_int_p_12b_lti() { t<int>(INCORESIZE/64, 16777216, 128); }
    void test_int_p_12b_ei()  { t<int>(INCORESIZE,    16777216, 128); }
    void test_int_p_12b_gti() { t<int>(INCORESIZE*2,  16777216, 128); }
    void test_int_p_n12b_lti() { t<int>(INCORESIZE/64, 16777216, 4096); }
    void test_int_p_n12b_ei()  { t<int>(INCORESIZE,    16777216, 4096); }
    void test_int_p_n12b_gti() { t<int>(INCORESIZE*2,  16777216, 4096); }

    typedef unsigned int uint;
    void test_uint_0_12b_lti() { t<uint>(INCORESIZE/64, 0, 64); }
    void test_uint_0_12b_ei()  { t<uint>(INCORESIZE,    0, 64); }
    void test_uint_0_12b_gti() { t<uint>(INCORESIZE*2,  0, 64); }
    void test_uint_0_n12b_lti() { t<uint>(INCORESIZE/64, 0, 4096); }
    void test_uint_0_n12b_ei()  { t<uint>(INCORESIZE,    0, 4096); }
    void test_uint_0_n12b_gti() { t<uint>(INCORESIZE*2,  0, 4096); }
    void test_uint_p_12b_lti() { t<uint>(INCORESIZE/64, 134217728, 16); }
    void test_uint_p_12b_ei()  { t<uint>(INCORESIZE,    134217728, 16); }
    void test_uint_p_12b_gti() { t<uint>(INCORESIZE*2,  134217728, 16); }
    void test_uint_p_n12b_lti() { t<uint>(INCORESIZE/64, 536870912, 4096); }
    void test_uint_p_n12b_ei()  { t<uint>(INCORESIZE,    536870912, 4096); }
    void test_uint_p_n12b_gti() { t<uint>(INCORESIZE*2,  536870912, 4096); }

    typedef boost::int64_t int64;
    void test_int64_n_12b_lti() { t<int64>(INCORESIZE/64, -8589934592, 8); }
    void test_int64_n_12b_ei()  { t<int64>(INCORESIZE,    -8589934592, 8); }
    void test_int64_n_12b_gti() { t<int64>(INCORESIZE*2,  -8589934592, 8); }
    void test_int64_n_n12b_lti() { t<int64>(INCORESIZE/64, -8589934592, 4096); }
    void test_int64_n_n12b_ei()  { t<int64>(INCORESIZE,    -8589934592, 4096); }
    void test_int64_n_n12b_gti() { t<int64>(INCORESIZE*2,  -8589934592, 4096); }
    void test_int64_0_12b_lti() { t<int64>(INCORESIZE/64, 0, 4); }
    void test_int64_0_12b_ei()  { t<int64>(INCORESIZE,    0, 4); }
    void test_int64_0_12b_gti() { t<int64>(INCORESIZE*2,  0, 4); }
    void test_int64_0_n12b_lti() { t<int64>(INCORESIZE/64, 0, 16384); }
    void test_int64_0_n12b_ei()  { t<int64>(INCORESIZE,    0, 16384); }
    void test_int64_0_n12b_gti() { t<int64>(INCORESIZE*2,  0, 16384); }
    void test_int64_p_12b_lti() { t<int64>(INCORESIZE/64, 17179869184, 32); }
    void test_int64_p_12b_ei()  { t<int64>(INCORESIZE,    17179869184, 32); }
    void test_int64_p_12b_gti() { t<int64>(INCORESIZE*2,  17179869184, 32); }
    void test_int64_p_n12b_lti() { t<int64>(INCORESIZE/64, 17179869184, 8192); }
    void test_int64_p_n12b_ei()  { t<int64>(INCORESIZE,    17179869184, 8192); }
    void test_int64_p_n12b_gti() { t<int64>(INCORESIZE*2,  17179869184, 8192); }

    typedef boost::uint64_t uint64;
    void test_uint64_0_12b_lti() { t<uint64>(INCORESIZE/64, 0, 256); }
    void test_uint64_0_12b_ei()  { t<uint64>(INCORESIZE,    0, 256); }
    void test_uint64_0_12b_gti() { t<uint64>(INCORESIZE*2,  0, 256); }
    void test_uint64_0_n12b_lti() { t<uint64>(INCORESIZE/64, 0, 16384); }
    void test_uint64_0_n12b_ei()  { t<uint64>(INCORESIZE,    0, 16384); }
    void test_uint64_0_n12b_gti() { t<uint64>(INCORESIZE*2,  0, 16384); }
    void test_uint64_p_12b_lti() { t<uint64>(INCORESIZE/64, 268435456, 256); }
    void test_uint64_p_12b_ei()  { t<uint64>(INCORESIZE,    268435456, 256); }
    void test_uint64_p_12b_gti() { t<uint64>(INCORESIZE*2,  268435456, 256); }
    void test_uint64_p_n12b_lti() { t<uint64>(INCORESIZE/64, 268435456, 4096); }
    void test_uint64_p_n12b_ei()  { t<uint64>(INCORESIZE,    268435456, 4096); }
    void test_uint64_p_n12b_gti() { t<uint64>(INCORESIZE*2,  268435456, 4096); }

    void test_float_n_12b_lti() { t<float>(INCORESIZE/64, -16384.4f, 32.6f); }
    void test_float_n_12b_ei()  { t<float>(INCORESIZE,    -16384.4f, 32.6f); }
    void test_float_n_12b_gti() { t<float>(INCORESIZE*2,  -16384.4f, 32.6f); }
    void test_float_n_n12b_lti() { t<float>(INCORESIZE/64,-50000.6f, 8168.2f);}
    void test_float_n_n12b_ei()  { t<float>(INCORESIZE,   -50000.7f, 8168.3f);}
    void test_float_n_n12b_gti() { t<float>(INCORESIZE*2, -50000.8f, 8168.7f);}
    void test_float_0_12b_lti() { t<float>(INCORESIZE/64, 0.0f, 39.6f); }
    void test_float_0_12b_ei()  { t<float>(INCORESIZE,    0.0f, 39.6f); }
    void test_float_0_12b_gti() { t<float>(INCORESIZE*2,  0.0f, 39.6f); }
    void test_float_0_n12b_lti() { t<float>(INCORESIZE/64, 0.0f, 32768.2f); }
    void test_float_0_n12b_ei()  { t<float>(INCORESIZE,    0.0f, 32768.3f); }
    void test_float_0_n12b_gti() { t<float>(INCORESIZE*2,  0.0f, 32768.7f); }
    void test_float_p_12b_lti() { t<float>(INCORESIZE/64, 123984.4f, 4.2f); }
    void test_float_p_12b_ei()  { t<float>(INCORESIZE,    123984.4f, 8.6f); }
    void test_float_p_12b_gti() { t<float>(INCORESIZE*2,  123984.4f, 22.2f); }
    void test_float_p_n12b_lti() { t<float>(INCORESIZE/64,123984.4f, 3456.7f);}
    void test_float_p_n12b_ei()  { t<float>(INCORESIZE,   123984.4f, 3456.7f);}
    void test_float_p_n12b_gti() { t<float>(INCORESIZE*2, 123984.4f, 3456.7f);}

    void test_double_n_12b_lti() { t<double>(INCORESIZE/64, -16384.4, 32.6); }
    void test_double_n_12b_ei()  { t<double>(INCORESIZE,    -16384.4, 32.6); }
    void test_double_n_12b_gti() { t<double>(INCORESIZE*2,  -16384.4, 32.6); }
    void test_double_n_n12b_lti() { t<double>(INCORESIZE/64,-50000.6, 8168.2);}
    void test_double_n_n12b_ei()  { t<double>(INCORESIZE,   -50000.7, 8168.3);}
    void test_double_n_n12b_gti() { t<double>(INCORESIZE*2, -50000.8, 8168.7);}
    void test_double_0_12b_lti() { t<double>(INCORESIZE/64, 0.0, 39.6); }
    void test_double_0_12b_ei()  { t<double>(INCORESIZE,    0.0, 39.6); }
    void test_double_0_12b_gti() { t<double>(INCORESIZE*2,  0.0, 39.6); }
    void test_double_0_n12b_lti() { t<double>(INCORESIZE/64, 0.0, 32768.2); }
    void test_double_0_n12b_ei()  { t<double>(INCORESIZE,    0.0, 32768.3); }
    void test_double_0_n12b_gti() { t<double>(INCORESIZE*2,  0.0, 32768.7); }
    void test_double_p_12b_lti() { t<double>(INCORESIZE/64, 123984.4, 4.2); }
    void test_double_p_12b_ei()  { t<double>(INCORESIZE,    123984.4, 8.6); }
    void test_double_p_12b_gti() { t<double>(INCORESIZE*2,  123984.4, 22.2); }
    void test_double_p_n12b_lti() { t<double>(INCORESIZE/64,123984.4, 3456.7);}
    void test_double_p_n12b_ei()  { t<double>(INCORESIZE,   123984.4, 3456.7);}
    void test_double_p_n12b_gti() { t<double>(INCORESIZE*2, 123984.4, 3456.7);}

    // Ridiculous cases: i.e. all the same value
    void test_byte_neg() { t_constant<byte>(INCORESIZE/64, -42); }
    void test_byte_0()   { t_constant<byte>(INCORESIZE/64,   0); }
    void test_byte_pos() { t_constant<byte>(INCORESIZE/64,  42); }
    void test_ubyte_0()   { t_constant<ubyte>(INCORESIZE/64,  0); }
    void test_ubyte_pos() { t_constant<ubyte>(INCORESIZE/64, 42); }
    void test_short_neg() { t_constant<short>(INCORESIZE/64, -5192); }
    void test_short_0()   { t_constant<short>(INCORESIZE/64,     0); }
    void test_short_pos() { t_constant<short>(INCORESIZE/64,  1296); }
    void test_int_neg() { t_constant<int>(INCORESIZE/64, -70000); }
    void test_int_0()   { t_constant<int>(INCORESIZE/64,      0); }
    void test_int_pos() { t_constant<int>(INCORESIZE/64,  52378); }
    void test_uint_0()   { t_constant<uint>(INCORESIZE/64,      0); }
    void test_uint_pos() { t_constant<uint>(INCORESIZE/64, 213897); }
    void test_int64_neg() { t_constant<int64>(INCORESIZE/64, -1389710); }
    void test_int64_0()   { t_constant<int64>(INCORESIZE/64,        0); }
    void test_int64_pos() { t_constant<int64>(INCORESIZE/64,  2314987); }
    void test_uint64_0()   { t_constant<uint64>(INCORESIZE/64,       0); }
    void test_uint64_pos() { t_constant<uint64>(INCORESIZE/64, 2938471); }
    void test_float_neg() { t_constant<float>(INCORESIZE/64, -981237.13f); }
    void test_float_0()   { t_constant<float>(INCORESIZE/64,        0.0f); }
    void test_float_pos() { t_constant<float>(INCORESIZE/64,  24197.936f); }
    void test_double_neg() { t_constant<double>(INCORESIZE/64, -4789612.12); }
    void test_double_0()   { t_constant<double>(INCORESIZE/64,         0.0); }
    void test_double_pos() { t_constant<double>(INCORESIZE/64,  14789612.2); }
};
