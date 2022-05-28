#include "parser.h"

int ParserIterator::load_batch_size = 1 * 1000 * 1000; // 1M records per batch

ParserIterator::~ParserIterator() {}
