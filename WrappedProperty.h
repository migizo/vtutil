/*
  ==============================================================================

    WrappedProperty.h
    Author:  migizo

  ==============================================================================
*/

#pragma once
#include <juce_data_structures/juce_data_structures.h>

namespace vtutil
{

class  PropertyListener
{
public:
    virtual ~PropertyListener() = default;
    virtual void propertyChanged(juce::ValueTree&, const juce::Identifier&) = 0;
};

/**
 @brief juce::ValueTreeのプロパティを静的型として扱うためのラッパークラス
 - 値を制限するコールバックを指定可能なため、最小最大値での制限や文字数制限などを行うことが可能。
 - juce::ValueTree::Listenerを使用する場合、想定の順番でリスナー関数が呼ばれないことがあったり取り扱いが難しいため、
 - このクラスの変更コールバックを使用することで値の制限などを行なった上で外部に通知することが可能。(そのためjuce::ValueTree::Listenerは非推奨)
 - プロパティと値を同期するため,プロパティのremove操作は行われない想定
 
 CachedValueとは以下の点で異なる
 [キャッシュ / 同期]
 - 前提としてjuce::ValueTreeではプロパティをjuce::NamedValueSetで保持しているため,プロパティ取得時はfor文で任意のキーを探す処理コストがかかる
 - CachedValueでは値にアクセスする時に直接プロパティを参照せず,キャッシュされた<Type>型の変数にアクセスすることでコストを下げている。
 - WrappedPropertyでは直接プロパティにアクセスするため前述のコストがかかる。ただし,ひとつのjuce::ValueTreeで管理するプロパティを少なくすることでこのコストは多少軽減可能
 [デフォルト状態の扱い]
 - CachedValueでのデフォルト状態は,対象プロパティが除外されているため,例えばresetToDefault()を呼び出した後にjuce::ValueTree::toXmlString()を呼ぶと,そのデフォルト状態のプロパティ値は含まれない
 - WrappedPropertyでのデフォルト状態は,対象プロパティを除外せずに指定のデフォルト値をプロパティに書き込む
 */

template <typename Type>
class WrappedProperty
: private juce::ValueTree::Listener
{
public:
//    using Listener = PropertyListener<WrappedProperty<Type>>;
    
    //! デフォルトコンストラクタ。紐付けされていないためreferTo()を呼び出す必要がある
    WrappedProperty() = default;
    
    WrappedProperty(juce::ValueTree& tree, const juce::Identifier& property, juce::UndoManager* um) { referTo(tree, property, um); }
    WrappedProperty(juce::ValueTree& tree, const juce::Identifier& property, juce::UndoManager* um, const Type& defaultVal) { referTo(tree, property, um, defaultVal); }
    ~WrappedProperty() = default;

    bool operator== (const Type& other) const { return get() == other.get(); }
    bool operator!= (const Type& other) const { return ! operator== (other); }
    inline WrappedProperty<Type>& operator= (const Type& newValue) { set(newValue); return *this; }
    
    Type get() const;
    void set(const Type& newValue);
    
    void referTo(juce::ValueTree& tree, const juce::Identifier& property, juce::UndoManager* um) { referTo(tree, property, um, defaultValue); }
    void referTo(juce::ValueTree& tree, const juce::Identifier& property, juce::UndoManager* um, const Type& defaultVal);

    void resetToDefault() { set(defaultValue); }
    void constrain(bool shouldConstrainWithDefault = true);
    void notifyChange();
    
    bool isValid() const { return targetTree.isValid() && targetProperty.isValid() && targetTree.hasProperty(targetProperty); }
    bool isTarget(const juce::ValueTree& tree, const juce::Identifier& property) const { return (property == targetProperty && tree == targetTree); }
    juce::Value getPropertyAsValue() { jassert(isValid()); return targetTree.getPropertyAsValue(targetProperty, undoManager); }
    
    juce::ValueTree& getValueTree() noexcept { return targetTree; }
    const juce::Identifier& getPropertyID() const noexcept { return targetProperty; }
    juce::UndoManager* getUndoManager() noexcept { return undoManager; }
    Type getDefault() const noexcept { return defaultValue; }
    
    void addListener(PropertyListener* l) { listenerList.add(l); }
    void removeListener(PropertyListener* l) { listenerList.remove(l); }

    std::function<void()> onChange = nullptr;
    std::function<void(Type&)> constrainer = nullptr;
    
private:
    void valueTreePropertyChanged(juce::ValueTree& changedTree, const juce::Identifier& changedProperty) override;
    void valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged) override;
    
    juce::ValueTree targetTree;
    juce::Identifier targetProperty;
    juce::UndoManager* undoManager = nullptr;
    Type defaultValue;
    bool ignoreCallback = false;
    
    juce::ListenerList<PropertyListener> listenerList;
};

//==============================================================================
// implementation
//==============================================================================
template <typename Type>
void WrappedProperty<Type>::referTo(juce::ValueTree& tree, const juce::Identifier& property, juce::UndoManager* um, const Type& defaultVal)
{
    jassert(tree.isValid());
    jassert(property.isValid());
    
    targetTree.removeListener (this);
    const Type lastVal = isValid() ? get() : defaultValue;
    targetTree = tree;
    targetProperty = property;
    undoManager = um;
    defaultValue = defaultVal;
    
    if (! targetTree.hasProperty(targetProperty))
    {
        auto defVal = juce::VariantConverter<Type>::toVar(defaultValue);
        targetTree.setProperty(targetProperty, defVal, undoManager);
    }
    
    constrain();
    if (lastVal != get()) notifyChange();

    if (isValid() == false)
    {
        jassertfalse;
        set(defaultValue);
    }

    targetTree.addListener (this);
}

template <typename Type>
Type WrappedProperty<Type>::get() const
{
    if (isValid())
        return juce::VariantConverter<Type>::fromVar(targetTree[targetProperty]);
    
    jassertfalse;
    return defaultValue;
}

template <typename Type>
void WrappedProperty<Type>::set(const Type& newValue)
{
    if (targetTree.isValid() && targetProperty.isValid())
    {
        targetTree.setProperty(targetProperty, juce::VariantConverter<Type>::toVar(newValue), undoManager);
        return;
    }
    
    jassertfalse;
    auto defVal = juce::VariantConverter<Type>::toVar(defaultValue);
    targetTree.setProperty(targetProperty, defVal, undoManager);
}

template <typename Type>
void WrappedProperty<Type>::constrain(bool shouldConstrainWithDefault)
{
    Type val = get();
    Type lastVal = val;
    if (constrainer) constrainer(val);
    if (val != lastVal)
    {
        set(val);
    }

    if (shouldConstrainWithDefault)
    {
        lastVal = defaultValue;
        if (constrainer) constrainer(defaultValue);
    }
}

template <typename Type>
void WrappedProperty<Type>::notifyChange()
{
    if (onChange) onChange();
    listenerList.call(&PropertyListener::propertyChanged, targetTree, targetProperty);
}

template <typename Type>
void WrappedProperty<Type>::valueTreePropertyChanged(juce::ValueTree& changedTree, const juce::Identifier& changedProperty)
{
    if (! isTarget(changedTree, changedProperty)) return;

    if (ignoreCallback) return;
    juce::ScopedValueSetter<bool> svs(ignoreCallback, true);
    
    jassert(isValid());
    
    // 値とプロパティを常に同期する想定のためpropertyの削除には対応しない
    if (targetTree.hasProperty(targetProperty) == false)
    {
        jassertfalse;
        set(defaultValue);
    }
    
    constrain(false);
    notifyChange();
}

template <typename Type>
void WrappedProperty<Type>::valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged)
{
    if (ignoreCallback) return;
    juce::ScopedValueSetter<bool> svs(ignoreCallback, true);
    
    // referTo時(でのlistener解除時)以外にリダイレクト(valueTreeに対する「=」を使用した参照先変更処理)は行わない想定
    jassertfalse;
    
    referTo(treeWhichHasBeenChanged, targetProperty, undoManager);
}

} // namespace vtutil
