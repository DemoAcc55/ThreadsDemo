#include <list>
#include <mutex>
#include <stack>
#include <thread>
#include <string>
#include <vector>
#include <memory>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <exception>

using namespace std;

/*
 * Примеры из книжки.
 * Для работы нужно указать флаг для компилятора "-pthread".
 * Инструкция для Code::Blocks здесь -
 *   https://askubuntu.com/questions/568068/multithreading-in-codeblocks
 *
 * Файл нечитабелен, зато всё в одном - легче запускать на разных машинах.
 */



/* Листинг 1.1 (стр 42) */
void hello()
{
    cout << "Hello Concurrent World" << endl;
}

// Запуск листинга (если надо запустить - вызываем в main(), далее - аналогично)
void run()
{
    thread t(hello);
    t.join();
}
/* Конец листинга 1.1 */



/* Листинг 2.1 (стр 47) */
void do_something(unsigned& j_)
{
    if (j_ % 100000 == 0) cout << "Value: " << j_ << endl;
}

struct func
{
    int& i;
    // Вырвиглазный конструктор с инициализацией поля
    func(int& i_):i(i_){}
    void operator()()
    {
        for (unsigned j = 0; j < 1000000; ++j)
        {
            do_something(j);
        }
    }
};

// Запуск листинга
void oops()
{
    // Ничего не выведет скорее всего, т.к.
    // detach отсоединяет поток от главного,
    // который сразу завершается
    int some_local_state = 0;
    func my_func(some_local_state);
    thread my_thread(my_func);
    my_thread.detach();
}
/* Конец листинга 2.1 */



/* Листинг 2.2 (стр 49) */
void do_something_in_current_thread()
{
    // Пустая функция, которая вызовется один раз
    cout << "do_something_in_current_thread() invoked" << endl;
}

// Запуск листинга
void f22()
{
    int some_local_state = 0;
    func my_func(some_local_state);
    thread t(my_func);
    // Далее джойним поток, чтобы он выполнился
    try
    {
        do_something_in_current_thread();
    }
    catch(...)
    {
        t.join();
        throw;
    }
    t.join();
}
/* Конец листинга 2.2 */



/* Листинг 2.3 (стр 50) */
class thread_guard
{
    thread& t;
public:
    // Оч некрасиво
    explicit thread_guard(thread& t_):t(t_){}
    // Деструктор, в котором гарантируется (ну почти), что поток завершится
    ~thread_guard()
    {
        if (t.joinable()) t.join();
    }
    thread_guard(thread_guard const&) = delete;
    thread_guard& operator=(thread_guard const&) = delete;
};

// Запуск листинга
void f23()
{
    int some_local_state = 0;
    func my_func(some_local_state);
    thread t(my_func);
    // Создаём экземпляр класса thread_guard,
    // который после выполнения этой функции самовыпилится, вызвав деструктор
    thread_guard g(t);
    do_something_in_current_thread();
}
/* Конец листинга 2.3 */



/* Листинг 2.4 (стр 52) */
enum cmd
{
    open_new_document,
    something_else
};

struct user_command
{
    cmd type;
public:
    user_command(cmd type_):type(type_){}
};

void open_document_and_display_gui(string const& filename)
{
    cout << "Open and display GUI for file " << filename << endl;
}

user_command get_user_input()
{
    int val;
    cout << "Enter value (0 or 1): ";
    cin >> val;
    return {static_cast<cmd>(open_new_document)};
}

string const get_filename_from_user()
{
    string new_name;
    cout << "Enter new name: ";
    cin >> new_name;
    return new_name;
}

bool done_editing()
{
    int done;
    cout << "Done editing (0|1)? ";
    cin >> done;
    return done == 1;
}

void process_user_input(user_command cmd)
{
    cout << "process_user_input(" << cmd.type << ") invoked" << endl;
}

// Запуск листинга
void edit_document(string const& filename)
{
    cout << "edit_document(" << filename << ") invoked" << endl;
    open_document_and_display_gui(filename);
    while (!done_editing())
    {
        user_command cmd = get_user_input();
        if (cmd.type == open_new_document)
        {
            string const new_name = get_filename_from_user();
            thread t(edit_document, new_name);
            t.detach();
        }
        else
        {
            process_user_input(cmd);
        }
    }
}
/* Конец листинга 2.4 */



/* Листинг 2.5 (стр 56) */
void some_function()
{
    cout << "some_function() invoked" << endl;
}

void some_other_function(int i)
{
    cout << "some_other_function(" << i << ") invoked" << endl;
}

thread f25()
{
    return thread(some_function);
}

thread g25()
{
    void some_other_function(int);
    thread t(some_other_function, 42);
    return t;
}

// Запуск листинга
void run25()
{
    f25().join();
    g25().join();
}
/* Конец листинга 2.5*/



/* Листинг 2.6 (стр 57) */
class scoped_thread
{
    thread t;
public:
    explicit scoped_thread(thread t_):t(move(t_))
    {
        if (!t.joinable())
            throw logic_error("No thread");
    }
    ~scoped_thread()
    {
        t.join();
    }
    scoped_thread(scoped_thread const&) = delete;
    scoped_thread& operator=(scoped_thread const&) = delete;
};

// Запуск листинга
void f26()
{
    int some_local_state;
    scoped_thread t{thread(func(some_local_state))};
    do_something_in_current_thread();
}
/* Конец листинга 2.6 */



/* Листинг 2.7 (стр 58) */
class joining_thread
{
    thread t;
public:
    joining_thread() noexcept = default;
    template<typename Callable, typename ... Args>
    explicit  joining_thread(Callable&& func, Args&& ... args):
        t(std::forward<Callable>(func), std::forward<Args>(args)...)
    {}
    explicit joining_thread(thread t_) noexcept:
        t(move(t_))
    {}
    joining_thread(joining_thread&& other) noexcept:
        t(move(other.t))
    {}
    joining_thread& operator=(joining_thread&& other) noexcept
    {
        if (joinable())
            join();
        t = move(other.t);
        return *this;
    }
    joining_thread& operator=(thread other) noexcept
    {
        if (joinable())
            join();
        t = move(other);
        return *this;
    }
    ~joining_thread() noexcept
    {
        if (joinable())
            join();
    }
    void swap(joining_thread& other) noexcept
    {
        t.swap(other.t);
    }
    thread::id get_id() const noexcept
    {
        return t.get_id();
    }
    bool joinable() const noexcept
    {
        return t.joinable();
    }
    void join()
    {
        t.join();
    }
    void detach()
    {
        t.detach();
    }
    const thread& as_thread() const noexcept
    {
        return t;
    }
};

// Запуск листинга
void run27()
{
    // Ничего не придумал, просто вызовем какую-нибудь функцию
    joining_thread jt(some_other_function, 55);
}
/* Конец листинга 2.7 */



/* Листинг 2.8 (стр 59) */
void do_work(unsigned id)
{
    cout << "do_work(" << id << ") invoked" << endl;
}

void f28()
{
    vector<thread> threads;
    for (unsigned i = 0; i < 20; ++i)
    {
        threads.emplace_back(do_work, i);
    }
    for (auto& entry : threads)
        entry.join();
}
/* Конец листинга 2.8 */



/* Листинг 2.9 (стр 60) */
template<typename Iterator, typename T>
struct accumulate_block
{
    void operator()(Iterator first, Iterator last, T& result)
    {
        result = accumulate(first, last, result);
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init)
{
    unsigned long const length = distance(first, last);
    if (!length)
        return init;
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads = thread::hardware_concurrency();
    unsigned long const num_threads = min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;
    vector<T> results(num_threads);
    vector<thread> threads(num_threads - 1);
    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_threads - 1); ++i)
    {
        Iterator block_end = block_start;
        advance(block_end, block_size);
        threads[i] = thread(accumulate_block<Iterator, T>(), block_start, block_end, ref(results[i]));
        block_start = block_end;
    }
    accumulate_block<Iterator, T>()(block_start, last, results[num_threads - 1]);
    for (auto& entry : threads)
        entry.join();
    return accumulate(results.begin(), results.end(), init);
}

// Запуск листинга
void run29()
{
    vector<int> v{1, 2, 3, 4};
    int result = parallel_accumulate(v.begin(), v.end(), 10);
    // возвращает 20, почему так - хз. разбираться лень
    cout << "result is " << result << endl;
}
/* Конец листинга 2.9 */



/* Листинг 3.1 (стр 71) */
list<int> some_list;
mutex some_mutex;

void add_to_list(int new_value)
{
    lock_guard<mutex> guard(some_mutex);
    some_list.push_back(new_value);
}

bool list_contains(int value_to_find)
{
    lock_guard<mutex> guard(some_mutex);
    return find(some_list.begin(), some_list.end(), value_to_find) != some_list.end();
}

// Запуск листинга
void run31()
{
    add_to_list(3);
    add_to_list(4);
    add_to_list(5);

    cout << "Is list contains 6? " << (list_contains(6) ? "yes" : "no") << endl;
    cout << "Is list contains 5? " << (list_contains(5) ? "yes" : "no") << endl;
}
/* Конец листинга 3.1 */



/* Листинг 3.2 (стр 72) */
class some_data
{
    int a;
    string b;
public:
    void do_something()
    {
        cout << "instance of some_data do_something()" << endl;
    }
};

class data_wrapper
{
private:
    some_data data;
    mutex m;
public:
    template<typename Function>
    void process_data(Function func)
    {
        lock_guard<mutex> l(m);
        func(data);
    }
};

some_data* unprotected;
void malicious_function(some_data& protected_data)
{
    unprotected = &protected_data;
}

data_wrapper x;
// Запуск листинга
void foo()
{
    x.process_data(malicious_function);
    unprotected->do_something();
}
/* Конец листинга 3.2 */



/* Листинг 3.3 (стр 74)
 * Это просто интерфейс без реализованных методов, смысла нет его делать.
 * Код добавляю, чтобы показать, что он хотя бы собирается.
 */
template<typename T, typename Container>
class stack33
{
public:
    explicit stack33(const Container&);
    explicit stack33(Container&& = Container());
    template<class Alloc> explicit stack33(const Alloc&);
    template<class Alloc> stack33(const Container&, const Alloc&);
    template<class Alloc> stack33(Container&&, const Alloc&);
    template<class Alloc> stack33(stack33&&, const Alloc&);
    bool empty() const;
    size_t size() const;
    T& top();
    T const& top() const;
    void push(T const&);
    void push(T&&);
    void pop();
    void swap(stack33&&);
    template<class... Args> void emplace(Args&&... args);
};
/* Конец листинга 3.3 */


/* Листинг 3.4 (стр 79)
 * Опять интерфейс. Собирается, пусть будет. Реализацию не делаю.
 */
struct empty_stack: exception
{
    const char* what() const noexcept;
};

template<typename T>
class threadsafe_stack
{
public:
    threadsafe_stack();
    threadsafe_stack(const threadsafe_stack&);
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;
    void push(T new_value);
    shared_ptr<T> pop();
    void pop(T& value);
    bool empty() const;
};
/* Конец листинга 3.4 */



/* Листинг 3.5 (стр 79) */
struct empty_stack_35: exception
{
    const char* what() const throw()
    {
        return "Empty stack exception";
    }
};

template<typename T>
class threadsafe_stack_35
{
private:
    stack<T> data;
    mutable mutex m;
public:
    threadsafe_stack_35(){}
    threadsafe_stack_35(const threadsafe_stack_35& other)
    {
        lock_guard<mutex> lock(other.m);
        data = other.data;
    }
    threadsafe_stack_35& operator=(const threadsafe_stack_35&) = delete;
    void push(T new_value)
    {
        lock_guard<mutex> lock(m);
        data.push(move(new_value));
    }
    shared_ptr<T> pop()
    {
        lock_guard<mutex> lock(m);
        if (data.empty()) throw empty_stack_35();
        shared_ptr<T> const res(make_shared<T>(data.top()));
        data.pop();
        return res;
    }
    void pop(T& value)
    {
        lock_guard<mutex> lock(m);
        if (data.empty()) throw empty_stack_35();
        value = data.top();
        data.pop();
    }
    bool empty() const
    {
        lock_guard<mutex> lock(m);
        return data.empty();
    }
};

// Запуск листинга
void run35()
{
    threadsafe_stack_35<int> st;
    st.push(1);
    st.push(3);
    st.push(2);

    int val;
    st.pop(val);

    cout << "Is stack empty? " << (st.empty() ? "yes" : "no") << endl;
    cout << "stack.pop() == " << val << endl;
}
/* Конец листинга 3.5 */

int main()
{
    cout << "main() invoked" << endl;
    return 0;
}
