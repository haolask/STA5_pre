set(hmac_SRCS
	hmacsha256.c
)

add_library(hmac OBJECT ${hmac_SRCS})
