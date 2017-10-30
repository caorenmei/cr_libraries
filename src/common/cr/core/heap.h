#ifndef CR_COMMON_HEAP_H_
#define CR_COMMON_HEAP_H_

#include <forward_list>
#include <functional>
#include <utility>
#include <vector>

namespace cr
{
    /** 堆特性萃取，比较和索引功能 */
    template <typename T>
    struct HeapTraits;

    /** 实现一个堆 */
    template <typename T, typename Traits = HeapTraits<T> >
    class Heap
    {
    public:

        /**
         * 构造函数
         * @param traits 堆特性
         */
        Heap(const Traits& traits = Traits());

        /**
         * 构造函数
         * @param first 起始迭代器
         * @param last 终止迭代器
         * @param traits 堆特性
         */
        template<typename InputIterator >
        Heap(InputIterator first, InputIterator last, const Traits& traits = Traits());

        /**
         * 构造函数
         * @param init 初始化列表
         * @param traits 堆特性
         */
        Heap(std::forward_list<T> init, const Traits& traits = Traits());

        /** 析构函数 */
        ~Heap();

        /**
         * 获取堆顶元素
         * @return 堆顶元素
         */
        const T& top() const;

        /**
         * 随机访问堆中的元素
         * @param index 元素索引
         * @return index 位置的元素
         */
        const T& operator[](std::size_t index) const;

        /**
         * 堆是否为空
         * @return true为空，false其他
         */
        bool empty() const;

        /**
         * 获取堆大小
         * @return 堆元素个数
         */
        std::size_t size() const;

        /**
         * 添加一个元素到堆中
         * @param value 待添加到堆的元素
         */
        void push(const T& value);

        /**
         * 添加一个元素到堆中
         * @param value 待添加到堆的元素
         */
        void push(T&& value);

        /**
         * 添加一个元素到堆中
         * @param args 堆元素的构造函数参数
         */
        template <typename... Args>
        void emplace(Args&&... args);

        /**
         * 弹出堆顶的元素
         */
        void pop();

        /**
         * 删除随机位置的元素
         * @param index 堆元素索引
         */
        void erase(std::size_t index);

        /**
         * 调增堆元素的位置,值在其他地方被改变
         * @param index 堆元素索引
         */
        void update(std::size_t index);

        /**
         * 设置堆元素的新值，并保证有序
         * @param index 堆元素索引
         * @param value 新的值
         */
        void update(std::size_t index, const T& value);

        /**
         * 设置堆元素的新值，并保证有序
         * @param index 堆元素索引
         * @param value 新的值
         */
        void update(std::size_t index, T&& value);

    private:

        void up(std::size_t index);

        void down(std::size_t index);

        void swap(std::size_t index1, std::size_t index2);

        std::vector<T> heap_;
        Traits traits_;
    };
}

template <typename T>
struct cr::HeapTraits
{
    void index(T& value, std::size_t index) const
    {}

    bool compare(const T& lhs, const T& rhs) const
    {
        return std::less<T>()(lhs, rhs);
    }
};

template <typename T, typename Traits>
cr::Heap<T, Traits>::Heap(const Traits& traits)
    : heap_(),
    traits_(traits)
{}

template <typename T, typename Traits>
template<typename InputIterator >
cr::Heap<T, Traits>::Heap(InputIterator first, InputIterator last, const Traits& traits = Traits())
    : Heap(traits)
{
    for (auto iter = first; iter != last; ++iter)
    {
        push(*iter);
    }
}

template <typename T, typename Traits>
cr::Heap<T, Traits>::Heap(std::forward_list<T> init, const Traits& traits = Traits())
    : Heap(init.begin(), init.end(), traits)
{}

template <typename T, typename Traits>
cr::Heap<T, Traits>::~Heap()
{}

template <typename T, typename Traits>
const T& cr::Heap<T, Traits>::top() const
{
    return heap_[0];
}

template <typename T, typename Traits>
const T& cr::Heap<T, Traits>::operator[](std::size_t index) const
{
    return heap_[index];
}

template <typename T, typename Traits>
bool cr::Heap<T, Traits>::empty() const
{
    return heap_.empty();
}

template <typename T, typename Traits>
std::size_t cr::Heap<T, Traits>::size() const
{
    return heap_.size();
}

template <typename T, typename Traits>
void cr::Heap<T, Traits>::push(const T& value)
{
    std::size_t index = heap_.size();
    heap_.push_back(value);
    traits_.index(heap_[index], index);
    up(index);
}

template <typename T, typename Traits>
void cr::Heap<T, Traits>::push(T&& value)
{
    std::size_t index = heap_.size();
    heap_.push_back(std::move(value));
    traits_.index(heap_[index], index);
    up(index);
}

template <typename T, typename Traits>
template <typename... Args>
void cr::Heap<T, Traits>::emplace(Args&&... args)
{
    std::size_t index = heap_.size();
    heap_.emplace_back(std::forward<Args>(args)...);
    traits_.index(heap_[index], index);
    up(index);
}

template <typename T, typename Traits>
void cr::Heap<T, Traits>::pop()
{
    erase(0);
}

template <typename T, typename Traits>
void cr::Heap<T, Traits>::erase(std::size_t index)
{
    if (index != heap_.size() - 1)
    {
        swap(index, heap_.size() - 1);
        heap_.pop_back();
        if (index > 0 && traits_.compare(heap_[index], heap_[(index - 1) / 2]))
        {
            up(index);
        }
        else
        {
            down(index);
        }
    }
    else
    {
        heap_.pop_back();
    }
}

template <typename T, typename Traits>
void cr::Heap<T, Traits>::update(std::size_t index)
{
    if (index > 0 && traits_.compare(heap_[index], heap_[(index - 1) / 2]))
    {
        up(index);
    }
    else
    {
        down(index);
    }
}

template <typename T, typename Traits>
void cr::Heap<T, Traits>::update(std::size_t index, const T& value)
{
    heap_[index] = value;
    update(index);
}

template <typename T, typename Traits>
void cr::Heap<T, Traits>::update(std::size_t index, T&& value)
{
    heap_[index] = std::move(value);
    update(index);
}

template <typename T, typename Traits>
void cr::Heap<T, Traits>::up(std::size_t index)
{
    while (index > 0)
    {
        std::size_t parent = (index - 1) / 2;
        if (!traits_.compare(heap_[index], heap_[parent]))
        {
            break;
        }
        swap(index, parent);
        index = parent;
    }
}

template <typename T, typename Traits>
void cr::Heap<T, Traits>::down(std::size_t index)
{
    std::size_t child = index * 2 + 1;
    while (child < heap_.size())
    {
        std::size_t minChild = (child + 1 == heap_.size()
            || traits_.compare(heap_[child], heap_[child + 1]))
            ? child : child + 1;
        if (traits_.compare(heap_[index], heap_[minChild]))
        {
            break;
        }
        swap(index, minChild);
        index = minChild;
        child = index * 2 + 1;
    }
}

template <typename T, typename Traits>
void cr::Heap<T, Traits>::swap(std::size_t index1, std::size_t index2)
{
    std::swap(heap_[index1], heap_[index2]);
    traits_.index(heap_[index1], index1);
    traits_.index(heap_[index2], index2);
}

#endif

