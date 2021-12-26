import redis

r = redis.Redis(host=redis_ip, port=redis_port, decode_responses=True)
for key in r.scan_iter("*"):
   r.delete(key)
