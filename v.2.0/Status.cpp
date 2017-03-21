#include "Status.h"

string Edge::type2String() {
	switch (tp)
	{
	case Edge::normal:
		return "normal";
	case Edge::head:
		return "head";
	case Edge::tail:
		return "tail";
	case Edge::check:
		return "check";
	case Edge::loop:
		return "loop";
	case Edge::capture:
		return "capture";
	case Edge::storage:
		return "storage";
	case Edge::end:
		return "end";
	case Edge::positive:
		return "positive";
	case Edge::negative:
		return "negative";
	default:
		return "NULL";
	}
}

string Edge::type2String(Edge::type t) {
	switch (t)
	{
	case Edge::normal:
		return "normal";
	case Edge::head:
		return "head";
	case Edge::tail:
		return "tail";
	case Edge::check:
		return "check";
	case Edge::loop:
		return "loop";
	case Edge::capture:
		return "capture";
	case Edge::storage:
		return "storage";
	case Edge::end:
		return "end";
	case Edge::positive:
		return "positive";
	case Edge::negative:
		return "negative";
	default:
		return "NULL";
	}
}
