from functools import wraps

from flask import jsonify, request


def require_api_key(expected_key: str):
    def decorator(func):
        @wraps(func)
        def wrapped(*args, **kwargs):
            provided = request.headers.get("X-API-Key", "")
            if expected_key and provided != expected_key:
                return jsonify({"error": "unauthorized"}), 401
            return func(*args, **kwargs)

        return wrapped

    return decorator
