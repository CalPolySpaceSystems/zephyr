zephyr_library()

zephyr_library_sources_ifdef(CONFIG_MS56XX ms56xx.c)