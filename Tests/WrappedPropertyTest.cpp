#include <gtest/gtest.h>
#include <vtutil/vtutil.h>

TEST(wrapped_property, default_constructor)
{
  using namespace juce;
  vtutil::WrappedProperty<String> wp;
  EXPECT_TRUE (wp.isUsingDefault());
  EXPECT_TRUE (wp.get() == String());
}

//! @brief デフォルト値を指定しない場合のテスト
TEST(wrapped_property, without_default_value)
{
  using namespace juce;
  ValueTree t ("root");
  t.setProperty ("testkey", "testvalue", nullptr);

  vtutil::WrappedProperty<String> cv (t, "testkey", nullptr);

  EXPECT_TRUE (! cv.isUsingDefault());
  EXPECT_TRUE (cv.get() == "testvalue");

  cv.resetToDefault();

  EXPECT_TRUE (cv.isUsingDefault());
  EXPECT_TRUE (cv.get() == String());
}

//! @brief デフォルト値を指定する場合のテスト
TEST(wrapped_property, with_default_value)
{
  using namespace juce;
  ValueTree t ("root");
  t.setProperty ("testkey", "testvalue", nullptr);

  vtutil::WrappedProperty<String> cv (t, "testkey", nullptr, "defaultvalue");

  EXPECT_TRUE (! cv.isUsingDefault());
  EXPECT_TRUE (cv.get() == "testvalue");

  cv.resetToDefault();

  EXPECT_TRUE (cv.isUsingDefault());
  EXPECT_TRUE (cv.get() == "defaultvalue");
  EXPECT_TRUE (! cv.isSyncPropertyWhenDefault());
  EXPECT_TRUE (! t.hasProperty("testkey"));

  cv.setSyncPropertyWhenDefault(true);
  EXPECT_TRUE (t.hasProperty("testkey"));
  EXPECT_TRUE ( t["testkey"].toString() == "defaultvalue");
}

//! @brief juce::var()がjuce::ValueTreeのプロパティにセットされていた場合のテスト
TEST(wrapped_property, with_void_value)
{
  using namespace juce;
  ValueTree t ("root");
  t.setProperty ("testkey", var(), nullptr);

  vtutil::WrappedProperty<String> cv (t, "testkey", nullptr, "defaultvalue");

  EXPECT_TRUE (! cv.isUsingDefault());
  EXPECT_TRUE (cv.get() == "");
  EXPECT_EQ (cv.get(), String());
}

TEST(wrapped_property, constrain_value)
{
  const juce::Range<float> range(0.0f, 1.0f);

  // juce::Range<>::contains()関数はendの値を含まないため,これに対応するものを用意
  auto contains = [range](auto v) { return range.getStart() <= v & v <= range.getEnd(); };

  juce::ValueTree vt("Root");
  vt.setProperty("num", 10.0f, nullptr);

  vtutil::WrappedProperty<float> cv(vt, "num", nullptr);

  // 変更コールバックに対してもconstrain処理がすでに適用されているはず
  cv.onChange = [&cv, contains]()
  {
    EXPECT_TRUE(contains(cv.get()));
    EXPECT_TRUE(contains(cv.getDefault()));
  };

  auto rangeConstrainer = [range](float& newValue, bool isDefault)
  {
    newValue = range.clipValue(newValue);
  };
  cv.setConstrainer(rangeConstrainer);  

  EXPECT_TRUE(contains(cv.get()));

  cv.setDefault(-1);

  EXPECT_TRUE(contains(cv.getDefault()));
}


