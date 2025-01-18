#include <gtest/gtest.h>
#include <vtwrapper/vtwrapper.h>

class CustomWrappedTree
: public vtwrapper::WrappedTree
{
public:
    CustomWrappedTree() = default;
    ~CustomWrappedTree() override = default;

    void wrapPropertiesAndChildren() override {}
};

TEST(unique_ptr, default_constructor)
{
    vtwrapper::UniquePtr<CustomWrappedTree> ptr;
    EXPECT_TRUE (ptr == nullptr);
}

TEST(unique_ptr, valid_tree)
{
    juce::ValueTree vt("root");
    vt.appendChild(juce::ValueTree("child"), nullptr);

    vtwrapper::UniquePtr<CustomWrappedTree> ptr;
    ptr.referTo(vt, "child", nullptr);
    EXPECT_TRUE (ptr != nullptr);
    EXPECT_TRUE (ptr->getTypeID() == juce::Identifier("child"));
    EXPECT_TRUE (ptr->getValueTree().hasType("child"));

}

TEST(unique_ptr, invalid_tree)
{
    juce::ValueTree vt("root");

    vtwrapper::UniquePtr<CustomWrappedTree> ptr;
    ptr.referTo(vt, "child", nullptr);
    EXPECT_TRUE (ptr == nullptr);
}

TEST(unique_ptr, change_tree)
{
    juce::ValueTree vt("root");

    vtwrapper::UniquePtr<CustomWrappedTree> ptr;
    
    ptr.referTo(vt, "child", nullptr);
    EXPECT_TRUE (ptr == nullptr);

    vt.appendChild(juce::ValueTree("child"), nullptr);

    ptr.referTo(vt, "child", nullptr);
    EXPECT_TRUE (ptr != nullptr);
    EXPECT_TRUE (ptr->getTypeID() == juce::Identifier("child"));
    EXPECT_TRUE (ptr->getValueTree().hasType("child"));
}

TEST(unique_ptr, reset)
{
    juce::ValueTree vt("root");

    vtwrapper::UniquePtr<CustomWrappedTree> ptr;
    ptr.referTo(vt, "root", nullptr);
    auto wt = new CustomWrappedTree();
    ptr.reset(wt); // CustomWrappedTree::wrap()が呼ばれる
    EXPECT_TRUE (ptr != nullptr);
    EXPECT_TRUE (wt->isValid());
    EXPECT_TRUE (vt.isValid());
}
