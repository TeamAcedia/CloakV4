#include "lambda.h"

std::unique_ptr<LambdaThread> runInThread(const std::function<void()> &fn, const std::string &thread_name)
{
    std::unique_ptr<LambdaThread> t(new LambdaThread(thread_name));
    t->m_fn = fn;
    t->start();
    return t;
}