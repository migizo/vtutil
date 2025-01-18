#include <gtest/gtest.h>
#include <vtutil/vtutil.h>

class CustomWrappedTree
: public vtutil::WrappedTree
{
public:
    CustomWrappedTree() = default;
    ~CustomWrappedTree() override = default;

    void wrapPropertiesAndChildren() override {}
};

TEST(wrapped_tree, default_constructor)
{
    CustomWrappedTree wt;
    EXPECT_FALSE (wt.isValid());
}

TEST(wrapped_tree, valid_target)
{
    CustomWrappedTree wt;
    juce::ValueTree vt("root");
    
    // デフォルトでは新規作成される
    wt.wrap(vt, "root", nullptr);
    EXPECT_TRUE (wt.isValid());
}

TEST(wrapped_tree, invalid_target)
{
    CustomWrappedTree wt;
    juce::ValueTree vt;
    
    // デフォルトでは新規作成される
    wt.wrap(vt, "root", nullptr);
    EXPECT_TRUE (wt.isValid());

    // 明示的に新規作成を行わないように指定
    bool allowCreationIfValid = false;
    wt.wrap(vt, "root", nullptr, allowCreationIfValid);
    EXPECT_FALSE (wt.isValid());
}

TEST(wrapped_tree, valid_child_target)
{
    CustomWrappedTree wt;
    juce::ValueTree vt("root");
    vt.appendChild(juce::ValueTree("child"), nullptr);
    
    wt.wrap(vt, "child", nullptr);
    EXPECT_TRUE (wt.isValid());

    // 明示的に子を対象外に指定
    bool allowChildWrapping = false;
    wt.wrap(vt, "child", nullptr, true, allowChildWrapping);
    EXPECT_FALSE (wt.isValid());
}

TEST(wrapped_tree, invalid_child_target)
{
    CustomWrappedTree wt;
    juce::ValueTree vt("root");
    
    // デフォルトでは子が新規作成される
    wt.wrap(vt, "child", nullptr);
    EXPECT_TRUE (wt.isValid());
    EXPECT_TRUE (vt.getNumChildren() == 1);

    // 子が作成されているので以降のテストのため削除 
    vt.removeChild(vt.getChildWithName("child"), nullptr); 

    // 明示的に新規作成を行わないように指定
    bool allowCreationIfInvalid = false;
    wt.wrap(vt, "child", nullptr, allowCreationIfInvalid); 
    EXPECT_FALSE (wt.isValid());
    EXPECT_TRUE (vt.getNumChildren() == 0);

    // 明示的に子を対象外に指定
    bool allowChildWrapping = false;
    wt.wrap(vt, "child", nullptr, true, allowChildWrapping);
    EXPECT_FALSE (wt.isValid());
    EXPECT_TRUE (vt.getNumChildren() == 0);
}

TEST(wrapped_tree, redirect)
{
    CustomWrappedTree wt;
    juce::ValueTree vt("root");

    wt.wrap(vt, "root", nullptr, false, false);
    EXPECT_TRUE (wt.isValid());
    EXPECT_TRUE (wt.getValueTree().getReferenceCount() == 2);

    // wrap呼び出し時に渡したValueTreeがリダイレクトされたとしてもWrappedTree内部ではリダイレクトされないはず
    vt = juce::ValueTree("other");
    EXPECT_TRUE (wt.isValid());
    EXPECT_TRUE (wt.getValueTree().getReferenceCount() == 1);

    // wrap()を呼び出すと内部のValueTreeも更新される
    wt.wrap(vt, "root", nullptr, false, false);
    EXPECT_FALSE (wt.isValid());
    EXPECT_TRUE (wt.getValueTree().getReferenceCount() == 0); // 無効なvalueTreeを持つため参照カウントは0のはず
}

TEST(wrapped_tree, redirect_child)
{
    CustomWrappedTree wt;
    juce::ValueTree vt("root");
    juce::ValueTree vtChild("child");
    vt.appendChild(vtChild, nullptr);

    bool allowChildWrapping = true;
    wt.wrap(vt, "child", nullptr, false, allowChildWrapping);
    EXPECT_TRUE (wt.isValid());
    EXPECT_TRUE (wt.getValueTree().getReferenceCount() == 3); // vt,vtChild,wtが参照しているため3

    // リダイレクトされたとしてもWrappedTree内部ではリダイレクトされないはず
    vtChild = juce::ValueTree("other");
    EXPECT_TRUE (wt.isValid());
    EXPECT_TRUE (wt.getValueTree().getReferenceCount() == 2); // vt,wtが参照しているため2

    // 親がリダイレクトされたとしてもWrappedTree内部ではリダイレクトされないはず
    vt = juce::ValueTree("other");
    EXPECT_TRUE (wt.isValid());
    EXPECT_TRUE (wt.getValueTree().getReferenceCount() == 1);

    // wrap()を呼び出すと内部のValueTreeも更新される
    wt.wrap(vt, "child", nullptr, false, false);
    EXPECT_FALSE (wt.isValid());
    EXPECT_TRUE (wt.getValueTree().getReferenceCount() == 0); // 無効なvalueTreeを持つため参照カウントは0のはず
}

TEST(wrapped_tree, invalid_copy)
{
    CustomWrappedTree wt;
    CustomWrappedTree wt2;

    wt.wrap(juce::ValueTree("root"), "root", nullptr, false);
    EXPECT_TRUE (wt.isValid());

    // 違うtypeの場合はコピーしない
    wt2.wrap(juce::ValueTree("other"), "other", nullptr);
    EXPECT_TRUE (wt2.isValid());
    wt.copyPropertiesAndChildrenFrom(wt2);
    EXPECT_TRUE (wt.isValid());
    EXPECT_TRUE (wt.getTypeID() == juce::Identifier("root"));

    // 無効なValueTreeの場合はコピーしない
    wt2.wrap(juce::ValueTree(), {}, nullptr, false);
    EXPECT_FALSE (wt2.isValid());
    wt.copyPropertiesAndChildrenFrom(wt2);
    EXPECT_TRUE (wt.isValid());
    EXPECT_TRUE (wt.getTypeID() == juce::Identifier("root"));
}