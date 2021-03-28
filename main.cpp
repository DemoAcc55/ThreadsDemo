#include <map>
#include <list>
#include <mutex>
#include <stack>
#include <queue>
#include <deque>
#include <chrono>
#include <future>
#include <thread>
#include <string>
#include <vector>
#include <memory>
#include <climits>
#include <numeric>
#include <utility>
#include <iostream>
#include <algorithm>
#include <exception>
#include <shared_mutex>
#include <condition_variable>

using namespace std;

/*
 * Примеры из книжки.
 * Для работы нужно указать флаг для компилятора "-pthread".
 * Ещё нужно указать -std=c++17, иначе не соберётся.
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



/* Листинг 3.6 (стр 82) */
// Реализацию класса some_big_object и метода swap(...) нам не показали,
// поэтому функцию запуска не буду добавлять
// добавлю только функцию do_something() в класс some_big_object - понадобится в 3.11
class some_big_object
{
public:
    void do_something()
    {
        cout << "some_big_object#do_something() invoked" << endl;
    }
};
void swap(some_big_object& lhs, some_big_object& rhs);
class X
{
private:
    some_big_object some_detail;
    mutex m;
public:
    X(some_big_object const& sd):some_detail(sd){}
    friend void swap(X& lhs, X& rhs)
    {
        if (&lhs == &rhs)
            return;
        lock(lhs.m, rhs.m);
        lock_guard<mutex> lock_a(lhs.m, adopt_lock);
        lock_guard<mutex> lock_b(rhs.m, adopt_lock);
        swap(lhs.some_detail, rhs.some_detail);
    }
};
/* Конец листинга 3.6 */



/* Листинг 3.8 (стр 89) (функции запуска нет) */
class hierarchial_mutex
{
    mutex internal_mutex;
    unsigned long const hierarchy_value;
    unsigned long previous_hierarchy_value;
    static thread_local unsigned long this_thread_hierarchy_value;
    void check_for_hierarchy_violation()
    {
        if (this_thread_hierarchy_value <= hierarchy_value)
        {
            throw logic_error("mutex hierarchy violated");
        }
    }
    void update_hierarchy_value()
    {
        previous_hierarchy_value = this_thread_hierarchy_value;
        this_thread_hierarchy_value = hierarchy_value;
    }
public:
    explicit hierarchial_mutex(unsigned long value):
        hierarchy_value(value),
        previous_hierarchy_value(0)
    {}
    void lock()
    {
        check_for_hierarchy_violation();
        internal_mutex.lock();
        update_hierarchy_value();
    }
    void unlock()
    {
        if (this_thread_hierarchy_value != hierarchy_value)
            throw logic_error("mutex hierarchy violated");
        this_thread_hierarchy_value = previous_hierarchy_value;
        internal_mutex.unlock();
    }
    bool try_lock()
    {
        check_for_hierarchy_violation();
        if (!internal_mutex.try_lock())
            return false;
        update_hierarchy_value();
        return true;
    }
};

thread_local unsigned long hierarchial_mutex::this_thread_hierarchy_value(ULONG_MAX);
/* Конец листинга 3.8 */



/* Листинг 3.7 (стр 87)
 * Не работает обез листинга 3.8, поэтому идёт не по порядку
 */
hierarchial_mutex high_level_mutex(10000);
hierarchial_mutex low_level_mutex(5000);
hierarchial_mutex other_mutex(6000);
int do_low_level_stuff()
{
    cout << "do_low_level_stuff() invoke (return 0)" << endl;
    return 0;
}
int low_level_func()
{
    lock_guard<hierarchial_mutex> lk(low_level_mutex);
    return do_low_level_stuff();
}
void high_level_stuff(int some_param)
{
    cout << "high_level_stuff(" << some_param << ") invoked" << endl;
}
void high_level_func()
{
    lock_guard<hierarchial_mutex> lk(high_level_mutex);
    high_level_stuff(low_level_func());
}
void thread_a()
{
    high_level_func();
}
void do_other_stuff()
{
    cout << "do_other_stuff() invoked" << endl;
}
void other_stuff()
{
    high_level_func();
    do_other_stuff();
}
void thread_b()
{
    lock_guard<hierarchial_mutex> lk(other_mutex);
    other_stuff();
}

// Запуск листинга
void run37()
{
    // падает с "mutex hierarchy violated", но в принципе работает
    thread_a();
    thread_b();
}
/* Конец листинга 3.7 */



/* Листинг 3.9 (стр 91) (похож на 3.6, тоже без функции запуска) */
class X2
{
private:
    some_big_object some_detail;
    mutex m;
public:
    X2(some_big_object const& sd):some_detail(sd){}
    friend void swap(X2& lhs, X2& rhs)
    {
        if (&lhs == &rhs)
            return;
        unique_lock<mutex> lock_a(lhs.m, defer_lock);
        unique_lock<mutex> lock_b(rhs.m, defer_lock);
        lock(lock_a, lock_b);
        swap(lhs.some_detail, rhs.some_detail);
    }
};
/* Конец листинга 3.9 */



/* Листинг 3.10 (стр 96) */
class Y
{
private:
    int some_detail;
    mutable mutex m;
    int get_detail() const
    {
        lock_guard<mutex> lock_a(m);
        return some_detail;
    }
public:
    Y(int sd):some_detail(sd){}
    friend bool operator==(Y const& lhs, Y const& rhs)
    {
        if (&lhs == &rhs)
            return true;
        int const lhs_value = lhs.get_detail();
        int const rhs_value = rhs.get_detail();
        return lhs_value == rhs_value;
    }
};

// Запуск листинга
void run310()
{
    Y y1(5);
    Y y2(5);
    Y y3(10);

    cout << "y1(5), y2(5), y3(10)" << endl;

    cout << "y1 == y2 ? " << (y1 == y2) << endl;
    cout << "y1 == y3 ? " << (y1 == y3) << endl;
    cout << "y2 == y3 ? " << (y2 == y3) << endl;
    cout << "y2 == y2 ? " << (y2 == y2) << endl;
}
/* Конец листинга 3.10 */



/* Листинг 3.11 (стр 98) */
shared_ptr<some_big_object> resource_ptr;
mutex resource_mutex;

// Запуск листинга
void foo311()
{
    unique_lock<mutex> lk(resource_mutex);
    if (!resource_ptr)
        resource_ptr.reset(new some_big_object());
    lk.unlock();
    resource_ptr->do_something();
}
/* Конец листинга 3.11 */



/* Листинг 3.12 (стр 99)
 * Какие-то непонятные объекты, либу не нашёл
 * Даже собирать не будем
 */

/*
class X3
{
private:
    connection_info connection_details;
    connection_handle connection;
    once_flag connection_init_flag;
    void open_connection()
    {
        connection = connection_manager.open(connection_details);
    }
public:
    X3(connection_info const& connection_details_):
        connection_details(connection_details_)
    {}
    void send_data(data_packet const& data)
    {
        call_once(connection_init_flag, &X3::open_connection, this);
        connection.send_data(data);
    }
    data_packet receive_data()
    {
        call_once(connection_init_flag, &X3::open_connection, this);
        return connectin.receive_data();
    }
}; */
/* Конец листинга 3.12 */



/* Листинг 3.13 (стр 102) */
class dns_entry
{
public:
    dns_entry(){}
};

class dns_cache
{
    map<string, dns_entry> entries;
    mutable shared_mutex entry_mutex;
public:
    dns_cache(){}
    dns_entry find_entry(string const& domain) const
    {
        cout << "find entry(" << domain << ") invoked" << endl;
        shared_lock<shared_mutex> lk(entry_mutex);
        map<string, dns_entry>::const_iterator const it = entries.find(domain);
        return (it == entries.end()) ? dns_entry() : it->second;
    }
    void update_or_add_entry(string const& domain, dns_entry const& dns_details)
    {
        cout << "update or add \"" << domain << "\"..." << endl;
        lock_guard<shared_mutex> lk(entry_mutex);
        entries[domain] = dns_details;
    }
};

// Запуск листинга
void run313()
{
    dns_cache dns_cache_inst;
    dns_cache_inst.update_or_add_entry("google.com", dns_entry());
    dns_cache_inst.update_or_add_entry("yandex.ru", dns_entry());
    dns_cache_inst.find_entry("ya.ru");
}
/* Конец листинга 3.13 */



/* Листинг 4.1 (стр 108)
 * Реализацию класса data_chunk нам не предложили, а там довольно много методов используется
 * К тому же есть ещё какие-то методы без реализации
 * Поэтому оставлю код под комментарием
 */

/*
mutex mut;
queue<data_chunk> data_queue;
condition_variable data_cond;
void data_preparation_thread()
{
    while (more_data_to_prepare())
    {
        data_chunk const data = prepare_data();
        {
            lock_guard<mutex> lk(mut);
            data_queue.push(data);
        }
        data_cond.notify_one();
    }
}
void data_processing_thread()
{
    while (true)
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, []{return !data_queue.empty();});
        data_chunk data = data_queue.front();
        data_queue.pop();
        lk.unlock();
        process(data);
        if (is_last_chunk(data)) break;
    }
}
*/
/* Конец листинга 4.1 */



/* Листинг 4.2 (стр 110)
 * Интерфейс опять...
 * Метода запуска, естественно, не будет
 */
template<class T, class Container = deque<T>>
class queue
{
public:
    explicit queue(const Container&);
    explicit queue(Container&& = Container());
    template<class Alloc> explicit queue(const Alloc&);
    template<class Alloc> queue(const Container&, const Alloc&);
    template<class Alloc> queue(Container&&, const Alloc&);
    template<class Alloc> queue(queue&&, const Alloc&);
    void swap(queue& q);
    bool empty() const;
    size_t size() const;
    T& front();
    const T& front() const;
    T& back();
    const T& back() const;
    void push(const T& x);
    void push(T&& x);
    void pop();
    template<class... Args> void emplace(Args&&... args);
};
/* Конец листинга 4.2 */



/* Листинг 4.3 (стр 111)
 * И снова интерфейс
 */
template<typename T>
class threadsafe_queue43
{
public:
    threadsafe_queue43();
    threadsafe_queue43(const threadsafe_queue43&);
    threadsafe_queue43& operator=(const threadsafe_queue43&) = delete;
    void push(T new_value);
    bool try_pop(T& value);
    shared_ptr<T> try_pop();
    void wait_and_pop(T& value);
    shared_ptr<T> wait_and_pop();
    bool empty() const;
};
/* Конец листинга 4.3 */



/* Листинг 4.4 (стр 112) */
template<typename T>
class threadsafe_queue44
{
private:
    mutex mut;
    std::queue<T> data_queue;
    condition_variable data_cond;
public:
    void push(T new_value)
    {
        lock_guard<mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }
    void wait_and_pop(T& value)
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        value = data_queue.front();
        data_queue.pop();
    }
};

// Опять загадочный data_chunk без реализации, закомментирую
/*
threadsafe_queue44<data_chunk> data_queue;
void data_preparation_thread()
{
    while (more_data_to_prepare())
    {
        data_chunk const data = prepare_data();
        data_queue.push(data);
    }
}
void data_processing_thread()
{
    while (true)
    {
        data_chunk data;
        data_queue.wait_and_pop(data);
        process(data);
        if (is_last_chunk(data)) break;
    }
}
*/
/* Конец листинга 4.4 */



/* Листинг 4.5 (стр 113) */
template<typename T>
class threadsafe_queue45
{
private:
    mutable mutex mut;
    std::queue<T> data_queue;
    condition_variable data_cond;
public:
    threadsafe_queue45(){}
    threadsafe_queue45(threadsafe_queue45 const& other)
    {
        lock_guard<mutex> lk(other.mut);
        data_queue = other.data_queue;
    }
    void push(T new_value)
    {
        lock_guard<mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }
    void wait_and_pop(T& value)
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        value = data_queue.front();
        data_queue.pop();
    }
    shared_ptr<T> wait_and_pop()
    {
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        shared_ptr<T> res(make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool try_pop(T& value)
    {
        lock_guard<mutex> lk(mut);
        if (data_queue.empty()) return false;
        value = data_queue.front();
        data_queue.pop();
        return true;
    }
    shared_ptr<T> try_pop()
    {
        lock_guard<mutex> lk(mut);
        if (data_queue.empty()) return shared_ptr<T>();
        shared_ptr<T> res(make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool empty() const
    {
        lock_guard<mutex> lk(mut);
        return data_queue.empty();
    }
};

// Запуск листинга
void run45()
{
    threadsafe_queue45<int> tq;
    tq.push(5);
    tq.push(10);
    tq.push(15);

    cout << "Is empty? " << (tq.empty()) << endl;
    cout << "push 5, 10, 15" << endl;
    cout << "Is empty? " << (tq.empty()) << endl;
    int pvar;
    tq.try_pop(pvar);
    cout << "try_pop(int): " << pvar << endl; // pvar == 5
}
/* Конец листинга 4.5 */

/* Листинг 4.6 (стр 117) */
int find_the_answer_to_ltuae()
{
    return 123;
}

// Запуск листинга
void run46()
{
    future<int> the_answer = async(find_the_answer_to_ltuae);
    do_other_stuff();
    cout << "The answer is " << the_answer.get() << endl;
}
/* Конец листинга 4.6 */

/* Листинг 4.7 (стр 117) */
// Опять интерфейсы и какие-то поля, всё в куче, ничего не понятно
// Закомментирую
/* struct X47
{
    void foo(int, string const&);
    string bar(string const&);
};
X47 x47;
auto f1 = async(&X47::foo, &x47, 42, "hello");
auto f2 = async(&X47::bar, x47, "goodbye");
struct Y47
{
    double operator()(double);
};
Y47 y47;
auto f3 = async(Y47(), 3.141);
auto f4 = async(ref(y47), 2.718);
X47 baz(X&);
async(baz, ref(x47));
class move_only
{
public:
    move_only();
    move_only(move_only&&);
    move_only(move_only const&) = delete;
    move_only& operator=(move_only&&);
    move_only& operator=(move_only const&) = delete;
    void operator()();
};
auto f5 = async(move_only()); */
/* Конец листинга 4.7 */

/* Листинг 4.8 (стр 119) */
// Интерфейс...
class packaged_task48
{
public:
    template<typename Callable>
    explicit packaged_task48(Callable&& f);
    future<string> get_future();
    void operator()(vector<char>*, int);
};
/* Конец листинга 4.8 */

/* Листинг 4.9 (стр 120) */
// Собирается, но реализаций для функций не предоставили
// Поэтому запускать не будем
mutex m49;
deque<packaged_task<void()>> tasks;
bool gui_shutdown_message_received()
{
    return true;
}
void get_and_process_gui_message(){}
void gui_thread()
{
    while (!gui_shutdown_message_received())
    {
        get_and_process_gui_message();
        packaged_task<void()> task;
        {
            lock_guard<mutex> lk(m49);
            if (tasks.empty()) continue;
            task = move(tasks.front());
            tasks.pop_front();
        }
        task();
    }
}
thread gui_bg_thread(gui_thread);
template<typename Func>
future<void> post_task_for_gui_thread(Func f)
{
    packaged_task<void()> task(f);
    future<void> res = task.get_future();
    lock_guard<mutex> lk(m49);
    tasks.push_back(move(task));
    return res;
}
/* Конец листинга 4.9 */

/* Листинг 4.10 (стр 122) */
// Откуда connection_set?
// Закомментирую, всё равно не собирается
/*
void process_connections(connection_set& connections)
{
    while (!done(connections))
    {
        for(connection_iterator
                connection = connections.begin(), end = connections.end();
            connection != end;
            ++connection)
        {
            if (connection->has_incoming_data())
            {
                data_packet data = connection->incoming();
                promise<payload_type>& p = connection->get_promise(data.id);
                p.set_value(data.payload);
            }
            if (connection->has_outgoing_data())
            {
                outgoing_packet data = connection->top_of_outgoing_queue();
                connection->send(data.payload);
                data.promise.set_value(true);
            }
        }
    }
} */
/* Конец листинга 4.10 */

/* Листинг 4.11 (стр 133) */
// Без функции запуска
condition_variable cv411;
bool done411;
mutex m411;
bool wait_loop()
{
    auto const timeout = chrono::steady_clock::now() + chrono::milliseconds(500);
    unique_lock<mutex> lk(m411);
    while (!done411)
    {
        if (cv411.wait_until(lk, timeout) == cv_status::timeout) break;
    }
    return done411;
}
/* Конец листинга 4.11 */

/* Листинг 4.12 (стр ) */
template<typename T>
list<T> sequential_quick_sort(list<T> input)
{
    if (input.empty()) return input;
    list<T> result;
    result.splice(result.begin(), input, input.begin());
    T const& pivot = *result.begin();
    auto divide_point = partition(input.begin(), input.end(),
                                  [&](T const& t){return t < pivot;});
    list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    auto new_lower(sequential_quick_sort(move(lower_part)));
    auto new_higher(sequential_quick_sort(move(input)));
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower);
    return result;
}

// Запуск
void run412()
{
    cout << "run412() invoked" << endl;
    list<int> l;
    l.push_back(3);
    l.push_back(8);
    l.push_back(1);
    cout << "before sort: ";
    for (list<int>::iterator i = l.begin(); i != l.end(); i++)
        cout << *i << " ";
    cout << endl;

    l = sequential_quick_sort(l);
    cout << "after sort: ";
    for (list<int>::iterator i = l.begin(); i != l.end(); i++)
        cout << *i << " ";
    cout << endl;
}
/* Конец листинга 4.12 */

/* Листинг 4.13 (стр 138) */
template<typename T>
list<T> parallel_quick_sort(list<T> input)
{
    if (input.empty()) return input;
    list<T> result;
    result.splice(result.begin(), input, input.begin());
    T const& pivot = *result.begin();
    auto divide_point = partition(input.begin(), input.end(), [&](T const& t){return t < pivot;});
    list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    future<list<T>> new_lower(async(&parallel_quick_sort<T>, move(lower_part)));
    auto new_higher(parallel_quick_sort(move(input)));
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower.get());
    return result;
}

// Запуск
void run413()
{
    cout << "run413() invoked" << endl;
    list<int> l;
    l.push_back(6);
    l.push_back(453);
    l.push_back(0);
    cout << "before sort: ";
    for (list<int>::iterator i = l.begin(); i != l.end(); i++)
        cout << *i << " ";
    cout << endl;

    l = sequential_quick_sort(l);
    cout << "after sort: ";
    for (list<int>::iterator i = l.begin(); i != l.end(); i++)
        cout << *i << " ";
    cout << endl;
}
/* Конец листинга 4.13 */

/* Листинг 4.14 (стр 140) */
// Чёт не собирается, ругается на type/value mismatch at arg 1 in template parameter list ...
/*
template<typename F, typename A>
future<result_of<F(A&&)>::type> spawn_task(F&& f, A&& a)
{
    typedef result_of<F(A&&)>::type result_type;
    packaged_task<result_type(A&&)> task(move(f));
    future<result_type> res(task.get_future());
    thread t(move(task), move(a));
    t.detach();
    return res;
} */
/* Конец листинга 4.14 */

/* Листинг 4.15 (стр 143) */
// Никаких messaging не нашёл, естественно не собирается
// Да и функций некоторых нет
// Закомментирую
/*struct card_inserted
{
    string account;
};
class atm
{
    messaging::receiver incoming;
    messaging::sender bank;
    messaging::sender interface_hardware;
    void (atm::*state)();
    string account;
    string pin;
    void waiting_for_card()
    {
        interface_hardware.send(display_enter_card());
        incoming.wait().handle<card_inserted>(
            [&](card_inserted const& msg)
                {
                    account = msg.account;
                    pin = "";
                    interface_hardware.send(display_enter_pin());
                    state = &atm::getting_pin;
                });
    }
    void getting_pin();
public:
    void run()
    {
        state = &atm::waiting_for_card;
        try
        {
            for(;;) (this->*state)();
        }
        catch(messaging::close_queue const&){}
    }
};*/
/* Конец листинга 4.15 */

/* Листинг 4.16 (стр 144) */
// Опять нет типов
/*
void atm::getting_pin()
{
    incoming.wait()
        .handle<digit_pressed>(
            [&](digit_pressed const& msg)
                {
                    unsigned const pin_length = 4;
                    pin += msg.digit;
                    if (pin.length() == pin_length)
                    {
                        bank.send(verify_pin(account, pin, incoming));
                        state = &atm::verifying_pin;
                    }
                })
        .handle<clear_last_pressed>(
            [&](clear_last_pressed const& msg)
                {
                    if (!pin.empty()) pin.resize(pin.length()-1);
                })
        .handle<cancel_pressed>(
            [&](cancel_pressed const& msg)
            {
                state = &atm::done_processing;
            });
}*/
/* Конец листинга 4.16 */

/* Листинг 4.17 (стр 147) */
// Без функции запуска, собирается и ладно
template<typename Func>
future<decltype(declval<Func>()())> spawn_async(Func&& func)
{
    promise<decltype(declval<Func>()())> p;
    auto res = p.get_future();
    thread t([p = move(p), f = decay_t<Func>(func)]()
             mutable
             {
                 try
                 {
                     p.set_value(f());
                 }
                 catch(...)
                 {
                     p.set_exception(current_exception());
                 }

             });
    t.detach();
    return res;
}
/* Конец листинга 4.17 */

/* Листинг 4.18 (стр 148) */
// Опять левые типы и неизвестные функции...
/*
void process_login(string const& username, string const& password)
{
    try
    {
        user_id const id = backend.authenticate_user(username, password);
        user_data const info_to_display = backend.request_current_info(id);
        update_display(info_to_display);
    }
    catch(exception& e)
    {
        display_error(e);
    }
}*/
/* Конец листинга 4.18 */

/* Листинг 4.19 (стр 148) */
// То же самое, что и в 4.18, только в обёртке под async
/*future<void> process_login(string const& username, string const& password)
{
    return async(launch::async, [=]()
        {
            try
            {
                user_id const id = backend.authenticate_user(username, password);
                user_data const info_to_display = backend.request_current_info(id);
                update_display(info_to_display);
            }
            catch (exception& e)
            {
                display_error(e);
            }
        });
}*/
/* Конец листинга 4.19 */

/* Листинг 4.20 (стр 149) */
// То же самое, что и в 4.18, только в другой обёртке (а-ля цепочка)
/*
future<void> process_login(string const& username, string const& password)
{
    return spawn_async([=]()
        {
            return backend.authenticate_user(username, password);
        }).then([](future<user_id> id)
        {
            return backend.request_current_info(id.get());
        }).then([](future<user_data> info_to_display)
        {
            try
            {
                update_display(info_to_display.get());
            }
            catch (exception& e)
            {
                display_error(e);
            }
        });
}*/
/* Конец листинга 4.20 */

/* Листинг 4.21 (стр ) */
/* Конец листинга 4.21 */

/* Листинг 4.22 (стр ) */
/* Конец листинга 4.22 */

/* Листинг 4.23 (стр ) */
/* Конец листинга 4.23 */

/* Листинг 4.24 (стр ) */
/* Конец листинга 4.24 */

/* Листинг 4.25 (стр ) */
/* Конец листинга 4.25 */

/* Листинг 4.26 (стр ) */
/* Конец листинга 4.26 */

/* Листинг 4.27 (стр ) */
/* Конец листинга 4.27 */

int main()
{
    cout << "main() invoked" << endl;
    return 0;
}
