grep LM_SET_ERR app/*.c master/*.c src/*.c server/*.c | sed 's/.*LM_SET_ERR...[a-z_]*, *[A-Z0_]*,// | sort | more

