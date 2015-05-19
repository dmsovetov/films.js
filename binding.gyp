{
    "targets":
    [
        {
            "target_name":  "hello",
            "sources":
            [
                "src/main.cc",
                "src/DataMining/statistics/LinearRegression.cpp",
                "src/DataMining/Cluster.cpp",
                "src/DataMining/recommendation/Similarity.cpp",
                "src/DataMining/recommendation/Recommender.cpp",
                "src/DataMining/Films.cpp",
                "src/DataMining/Mongo.cpp",
            ],
            "include_dirs":
            [
                "src/DataMining",
                "src/libmongo/include/libbson-1.0",
                "src/libmongo/include/libmongoc-1.0",
            ],
            "xcode_settings": {
                "OTHER_CFLAGS":     [ "-std=gnu++11", "-std=c++11", "-Os", "-stdlib=libc++", "-mmacosx-version-min=10.10" ],
            },
            "cflags_cc": [ "-std=gnu++11", "-std=c++11", "-Os" ],
            'link_settings': {
                "ldflags":   [ '-L/usr/local/lib' ],
                "libraries": [ '/usr/local/lib/libmongoc-1.0.a', '/usr/local/lib/libbson-1.0.a' ],
            }
        }
    ]
}