
#include "header.h"
#include "plugin/Stitching_Grid.h"


class ImagjStitching {
public:
    static int func_grid(int type, std::vector<std::string> args, std::vector<std::string>  files) {

        // init log

        LOGINFO(args.size());
        for (auto& arg : args)
            LOGINFO(arg);

        if (args.size() < 8)
        {
            LOGINFO("Error: The number of parameters is less than 8");
            return 1;
        }

        if (files.size() < 2)
        {
            LOGINFO("Error: The number of parameters is less than 2");
            return 1;
        }

        LOGINFO("func_grid load start");
        Stitching_Grid sg;
        sg.run(args, files);
        LOGINFO("func_grid load success");
        return 0;
    }
};
