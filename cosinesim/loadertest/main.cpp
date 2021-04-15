
#ifdef HYBRID_DEMO
#define API_INLINE_IMPL
#include "api_loader.cpp"
#endif

#include "api.hpp"

Api *makeApi() {
    Options options;
    options.intOpt = 10;
    options.strOpt = "hello world this is a very long string to make sure that it uses dynamic space instead of static";
    Api *pApi = new Api(options);
    return pApi;
}

int main(int, char **) {
    Api *pApi = makeApi();
    pApi->func();
    return 0;
}
