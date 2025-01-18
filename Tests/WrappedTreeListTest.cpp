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

TEST(wrapped_tree_list, default_constructor)
{
    vtwrapper::WrappedTreeList<CustomWrappedTree> wtl;
    EXPECT_FALSE (wtl.isValid());
}

TEST(wrapped_tree_list, valid_target)
{
    vtwrapper::WrappedTreeList<CustomWrappedTree> wtl;
    juce::ValueTree vt("root");
    
    // デフォルトでは新規作成される
    wtl.wrap(vt, "root", "child", nullptr);
    EXPECT_TRUE (wtl.isValid());
}

TEST(wrapped_tree_list, invalid_target)
{
    vtwrapper::WrappedTreeList<CustomWrappedTree> wtl;
    juce::ValueTree vt;
    
    // デフォルトでは新規作成される
    wtl.wrap(vt, "root", "child", nullptr);
    EXPECT_TRUE (wtl.isValid());

    // 明示的に新規作成を行わないように指定
    bool allowCreationIfValid = false;
    wtl.wrap(vt, "root", "child", nullptr, allowCreationIfValid);
    EXPECT_FALSE (wtl.isValid());
}

TEST(wrapped_tree_list, change_tree)
{
    juce::ValueTree vt("root");

    vtwrapper::WrappedTreeList<CustomWrappedTree> wtl;
    
    wtl.wrap(vt, "root", "child", nullptr);
    EXPECT_TRUE (wtl.isValid());
    EXPECT_TRUE (wtl.isEmpty());

    juce::ValueTree vtChild("child");
    vt.appendChild(vtChild, nullptr);

    EXPECT_TRUE (wtl.size() == 1);
    EXPECT_TRUE (wtl.getFirst()->getValueTree().hasType(juce::Identifier("child")));

    vt.removeChild(vtChild, nullptr);
    EXPECT_TRUE (wtl.isEmpty());
}

TEST(wrapped_tree_list, change_list)
{
    juce::ValueTree vt("root");

    vtwrapper::WrappedTreeList<CustomWrappedTree> wtl;
    
    wtl.wrap(vt, "root", "child", nullptr);
    EXPECT_TRUE (wtl.isValid());
    EXPECT_TRUE (wtl.isEmpty());

    wtl.add(new CustomWrappedTree());
    wtl.add(new CustomWrappedTree());
    EXPECT_TRUE (wtl.size() == 2);
    EXPECT_TRUE (wtl.getFirst()->getValueTree().hasType(juce::Identifier("child")));

    wtl.remove(wtl.getFirst());
    wtl.remove(wtl.getFirst());
    EXPECT_TRUE (wtl.isEmpty());
}
