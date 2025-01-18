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

/**
 @brief juce::ValueTreeのプロパティを静的型として扱うためのラッパークラス
 - 値を制限するコールバックを指定可能なため、最小最大値での制限や文字数制限などを行うことが可能。
 - juce::ValueTree::Listenerを使用する場合、想定の順番でリスナー関数が呼ばれないことがあったり取り扱いが難しいため、 @n
 このクラスの変更コールバックを使用することで値の制限などを行なった上で外部に通知することが可能。(そのためjuce::ValueTree::Listenerは非推奨)
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
    //! デフォルトコンストラクタ。紐付けされていないためreferTo()を呼び出す必要がある
    WrappedProperty() = default;
    WrappedProperty(juce::ValueTree& tree, const juce::Identifier& property, juce::UndoManager* um) { referTo(tree, property, um); }
    WrappedProperty(juce::ValueTree& tree, const juce::Identifier& property, juce::UndoManager* um, const Type& defaultVal) { referTo(tree, property, um, defaultVal); }
    ~WrappedProperty() override = default;

    bool operator== (const WrappedProperty<Type>& other) const { return get() == other.get(); }
    bool operator!= (const WrappedProperty<Type>& other) const { return ! operator== (other); }
    bool operator== (const Type& other) const { return get() == other; }
    bool operator!= (const Type& other) const { return ! operator== (other); }
    inline WrappedProperty<Type>& operator= (const Type& newValue) { set(newValue); return *this; }
    
    Type get() const;
    void set(Type newValue);
    
    void referTo(juce::ValueTree& tree, const juce::Identifier& property, juce::UndoManager* um) { referTo(tree, property, um, defaultValue); }
    void referTo(juce::ValueTree& tree, const juce::Identifier& property, juce::UndoManager* um, const Type& defaultVal);

    void resetToDefault() { set(defaultValue); }
    
    void setDefault(const Type& defaultVal);
    void setConstrainer(std::function<void(Type& newValue, bool isDefault)> newConstrainer);

    //! @brief デフォルト値の場合にjuce::ValueTreeのプロパティにもデフォルト値として保持しておくかどうか。
    //! @param shouldSync trueでは常にjuce::ValueTreeのプロパティとして保持され、falseではデフォルト値の場合にjuce::ValueTreeのプロパティから削除される。
    //! @n falseの場合はjuce::CachedValue<>と同じ挙動である。デフォルトではfalseが指定されている。
    void setSyncPropertyWhenDefault(bool shouldSync);
    bool isSyncPropertyWhenDefault() const { return syncPropertyWhenDefault; }

    bool isValid() const { return targetTree.isValid() && targetProperty.isValid(); }
    juce::Value getPropertyAsValue() { jassert(isValid()); return targetTree.getPropertyAsValue(targetProperty, undoManager); }
    bool isUsingDefault() const { return getDefault() == get(); }

    juce::ValueTree& getValueTree() noexcept { return targetTree; }
    const juce::Identifier& getPropertyID() const noexcept { return targetProperty; }
    juce::UndoManager* getUndoManager() noexcept { return undoManager; }
    Type getDefault() const noexcept { return defaultValue; }

    std::function<void()> onChange = nullptr;
    
private:
    void valueTreePropertyChanged(juce::ValueTree& changedTree, const juce::Identifier& changedProperty) override;
    void valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged) override;
    
    juce::ValueTree targetTree;
    juce::Identifier targetProperty;
    juce::UndoManager* undoManager = nullptr;
    Type defaultValue;
    Type cachedValue;
    bool ignoreCallback = false;
    bool syncPropertyWhenDefault = false;
    std::function<void(Type& newValue, bool isDefault)> constrainer = nullptr;
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
    
    targetTree = tree;
    targetProperty = property;
    undoManager = um;
    defaultValue = defaultVal;
    valueTreePropertyChanged(targetTree, targetProperty);

    targetTree.addListener (this);
}

template <typename Type>
Type WrappedProperty<Type>::get() const
{
    return cachedValue;
}

template <typename Type>
void WrappedProperty<Type>::set(Type newValue)
{
    if (! isValid())
    {
        jassertfalse;
        cachedValue = newValue;
        return;
    }
    
    // デフォルト値同期offかつデフォルト値と同じ値の場合にプロパティ削除
    if (! syncPropertyWhenDefault && newValue == defaultValue)
    {
        targetTree.removeProperty(targetProperty, undoManager);
    }
    // それ以外はpropertyをセット
    else
    {
        if (constrainer) 
            constrainer(newValue, false);
        
        targetTree.setProperty(targetProperty, juce::VariantConverter<Type>::toVar(newValue), undoManager);
    }
}

template <typename Type>
void WrappedProperty<Type>::setDefault(const Type& newDefaultVal)
{
    defaultValue = newDefaultVal;
    if (constrainer) constrainer(defaultValue, true);

    if (! isValid())
    {
        jassertfalse;
        return;
    }
    
    // デフォルト同期offの場合に、既にあるプロパティが新しいデフォルト値と同じだった場合に削除する
    if (! syncPropertyWhenDefault && cachedValue == defaultValue)
    {
        targetTree.removeProperty(targetProperty, undoManager);
    }
}

template <typename Type>
void WrappedProperty<Type>::setConstrainer(std::function<void(Type& newValue, bool isDefault)> newConstrainer)
{
    constrainer = newConstrainer;
    setDefault(defaultValue);
    set(cachedValue);
}

template <typename Type>
void WrappedProperty<Type>::setSyncPropertyWhenDefault(bool shouldSync)
{
    if (syncPropertyWhenDefault == shouldSync) return;
    syncPropertyWhenDefault = shouldSync;
    set(cachedValue);
}

//==============================================================================
template <typename Type>
void WrappedProperty<Type>::valueTreePropertyChanged(juce::ValueTree& changedTree, const juce::Identifier& changedProperty)
{
    if (ignoreCallback) return;
    juce::ScopedValueSetter<bool> svs(ignoreCallback, true);
    
    if (changedTree != targetTree || changedProperty != targetProperty) return;
    if (! isValid())
    {
        jassertfalse;
        return;
    }
    
    auto lastValue = cachedValue;
        
    // デフォルト同期offの場合にproperty削除された場合はキャッシュ値をデフォルトにする
    if (!syncPropertyWhenDefault && targetTree.hasProperty(targetProperty) == false)
        cachedValue = defaultValue;
    else if (targetTree.hasProperty(targetProperty))
        cachedValue = juce::VariantConverter<Type>::fromVar(targetTree[targetProperty]);
    // デフォルト同期off以外でproperty削除されることは想定されていない
    else
        jassertfalse;
    
    if (constrainer) 
    {
        constrainer(cachedValue, false);
        if (targetTree.hasProperty(targetProperty))
        {
            targetTree.setPropertyExcludingListener(this, targetProperty, juce::VariantConverter<Type>::toVar(cachedValue), undoManager);
        }
    }
    if (lastValue != cachedValue && onChange) onChange();
}

template <typename Type>
void WrappedProperty<Type>::valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged)
{
    if (ignoreCallback) return;
    juce::ScopedValueSetter<bool> svs(ignoreCallback, true);
    
    referTo(treeWhichHasBeenChanged, targetProperty, undoManager);
}

} // namespace vtutil
