#include <limits>
#include "qe.h"

/*
 *
 * Filter
 *
 */

Filter::Filter(Iterator *input, const Condition &condition) {
    this->input = input;
    this->condition = condition;
}

// ... the rest of your implementations go here

RC Filter::getNextTuple(void *data){
    // filters the tuples from the input iterator by applying the filter predicate condition on them.

    // we need to make sure that we call the getNextTuple first before judge it.
    do{
        // not-satisfy the condition, we call the getNextTuple() until we get the satisfied result or we end.
        if(input->getNextTuple(data) == QE_EOF){
            // End of scan.
//            std::cout << "[Filter] end of scan in getNextTuple()." << std::endl;
            return QE_EOF;
        }
    }while(!isSatisfied(data));

    // satisfy the condition:
    return 0;
}

void Filter::getAttributes(std::vector<Attribute> &attrs) const {
    input->getAttributes(attrs);
}

bool Filter::isSatisfied(void *data) {

    if(condition.bRhsIsAttr){
        // std::cerr << "[Error] invalid rhs value. " << std::endl;
        return -1;
    }

    std::vector<Attribute> lhsallAttributes;
    input->getAttributes(lhsallAttributes);

    // we get the attrType according to rhs
    attrType = condition.rhsValue.type;
//    std::cout << "attrType is" << attrType << std::endl;
    
    void *selData = malloc(MAX_TUPLE_LEN);
    std::vector<std::string> selAttrs;
    selAttrs.push_back(condition.lhsAttr);
    int nullIndicatorSize = ceil(double(selAttrs.size())/CHAR_BIT);
    extractFromReturnedData(lhsallAttributes, selAttrs, data, selData);
    
    RC rc = compLeftRightVal(this->attrType, this->condition, selData, condition.rhsValue.data, nullIndicatorSize, 0);
    
    free(selData);
    
    return rc;
}

/*
 *
 * Project
 *
 */

Project::Project(Iterator *input, const std::vector<std::string> &attrNames) {
    this->ite_input = input;
    this->attrNames = attrNames;

    // initialize allAttrs and projectAttrs
    ite_input->getAttributes(allAttrs);

    // find the matching attributes
    for(auto & allAttr : allAttrs){
        for(const auto & attrName : attrNames){
            if(allAttr.name == attrName){
                projectAttrs.emplace_back(allAttr);
                // we only could have one chance.
                break;
            }
        }
    }
}

RC Project::getNextTuple(void *data) {

    // first, we get the whole tuple.
    void *tuple = malloc(MAX_TUPLE_LEN);
    memset(tuple, 0, MAX_TUPLE_LEN);
    
    if(ite_input->getNextTuple(tuple) != 0)
    {
        free(tuple);
        return QE_EOF;
    }

    extractFromReturnedData(this->allAttrs, this->attrNames, tuple, data);
    free(tuple);
    
    return 0;
}

void Project::getAttributes(std::vector<Attribute> &attrs) const {
    attrs.clear();
    attrs = projectAttrs;
}

/*
 *
 * BNLJoin
 *
 */

BNLJoin::BNLJoin(Iterator *leftIn, TableScan *rightIn, const Condition &condition, const unsigned numPages) {
    this->leftIn = leftIn;
    this->rightIn = rightIn;
    this->condition = condition;
    this->numPageinBlock = numPages;
    this->lhsTupleData = nullptr;
    this->rhsTupleData = nullptr;
    this->leftTableisOver = false;
    this->isFirstTime = true;
    this->restart = false;
    this->lhsOffsetIndex = 0;
    this->rhsOffsetIndex = 0;

    this->leftIn->getAttributes(lhsAttributes);
    this->rightIn->getAttributes(rhsAttributes);

}

void BNLJoin::getAttributes(std::vector<Attribute> & attrs) const {
    attrs.clear();
    // add lhs and rhs
    for(auto& i : lhsAttributes){
        attrs.emplace_back(i);
    }

    for(auto& j : rhsAttributes){
        attrs.emplace_back(j);
    }
}


RC BNLJoin::getNextTuple(void *data) {
    if(!condition.bRhsIsAttr){
        // not the join.
        // std::cerr << "[Error] Right Attribute is not the join situation. " << std::endl;
        return -1;
    }

    if(isFirstTime){
        // initialize
        isFirstTime = false;

        rhsTupleData = malloc(MAX_TUPLE_LEN);
        if(rightIn->getNextTuple(rhsTupleData) == QE_EOF){
            // std::cerr << "No data in right table. " << std::endl;
            return -1;
        }

        rhsOffsetIndex = ceil((double(rhsAttributes.size())/CHAR_BIT));
        for(auto &attr : rhsAttributes){
            int stepLen = 0;
            if(attr.type == TypeVarChar){
                // we need to add the varchar length to the offset
                int varCharLen = 0;
                memcpy(&varCharLen, (char*)rhsTupleData + rhsOffsetIndex, 4);
                stepLen += varCharLen;
            }

            stepLen += 4;

            if(attr.name == condition.rhsAttr){
                // matched
                joinTargetType = attr.type;
                // std::cout << "[Warning] BNL Join target attribute is set: " << attr.name << std::endl;
                break;
            }

            rhsOffsetIndex += stepLen;
        }

        loadBlockBuffer(true);
    }

    // not the first time, do the comparison and check status
    while(!isBNLJoinSatisfied()){
        if(joinTargetType == TypeInt){
            // first, we need to check whether the hashmap is empty or full
            if(intCurrentPos == intBlockBuffer.end() && !leftTableisOver){
                // we have reached the last element of hashmap and we should clean and reload.
                loadBlockBuffer(false);
                continue;
            }

            if(intCurrentPos == intBlockBuffer.end() && leftTableisOver){
                // Finish the first round, we need to update both the left and right.
                // TO DO: re-start scan of left Table
                restart = true;
                it = leftTable.begin();

                loadBlockBuffer(false);

                memset(rhsTupleData, 0, MAX_TUPLE_LEN);
                if(rightIn->getNextTuple(rhsTupleData) == QE_EOF){
                    if(leftTableisOver){
                        // finish the join.
                        // std::cout << "Finish BNL join. " << std::endl;
                        return -1;
                    }
                    else{
                        rightIn->setIterator();
                        rightIn->getNextTuple(rhsTupleData);
                    }
                }

                continue;
            }

            intCurrentPos++;
        }
        else if(joinTargetType == TypeReal){
            // first, we need to check whether the hashmap is empty or full
            if(floatCurrentPos == floatBlockBuffer.end() && !leftTableisOver){
                // we have reached the last element of hashmap and we should clean and reload.
                loadBlockBuffer(false);
                continue;
            }

            if(floatCurrentPos == floatBlockBuffer.end() && leftTableisOver){
                // Finish the first round, we need to update both the left and right.
                // TO DO: re-start scan of left Table
                restart = true;
                it = leftTable.begin();
                loadBlockBuffer(false);
                memset(rhsTupleData, 0, MAX_TUPLE_LEN);
                if(rightIn->getNextTuple(rhsTupleData) == QE_EOF){
                    // finish the join.
                    // std::cout << "Finish BNL join. " << std::endl;
                    return -1;
                    // rightIn->setIterator();
                    // rightIn->getNextTuple(rhsTupleData);
                }

                continue;
            }

            floatCurrentPos++;
        }
        else{
            // first, we need to check whether the hashmap is empty or full
            if(varcharCurrentPos == varcharBlockBuffer.end() && !leftTableisOver){
                // we have reached the last element of hashmap and we should clean and reload.
                loadBlockBuffer(false);
                continue;
            }

            if(varcharCurrentPos == varcharBlockBuffer.end() && leftTableisOver){
                // Finish the first round, we need to update both the left and right.
                // TO DO: re-start scan of left Table
                restart = true;
                it = leftTable.begin();
                loadBlockBuffer(false);
                memset(rhsTupleData, 0, MAX_TUPLE_LEN);
                if(rightIn->getNextTuple(rhsTupleData) == QE_EOF){
                    // finish the join.
                    // std::cout << "Finish BNL join. " << std::endl;
                    return -1;
                    // rightIn->setIterator();
                    // rightIn->getNextTuple(rhsTupleData);
                }

                continue;
            }

            varcharCurrentPos++;
        }
    }

    // current satisfy the join, we concatenate them and output.
    std::vector<Attribute> allAttrs;
    this->getAttributes(allAttrs);

    if (joinTargetType == TypeInt) {
        concatenateData(allAttrs, lhsAttributes, rhsAttributes, intCurrentPos->second, rhsTupleData, data);
        intCurrentPos++;
    } else if (joinTargetType == TypeReal) {
        concatenateData(allAttrs, lhsAttributes, rhsAttributes, floatCurrentPos->second, rhsTupleData, data);
        floatCurrentPos++;
    } else {
        // varchar
        concatenateData(allAttrs, lhsAttributes, rhsAttributes, varcharCurrentPos->second, rhsTupleData, data);
        varcharCurrentPos++;
    }

    return 0;
}

RC BNLJoin::loadBlockBuffer(bool isFirst) {

    // clean first.
    cleanBlockBuffer();

    // call the left.getNextTuple to load tuples into the map until the buffer is full.
    int totalTupleLen = 0;

    if(isFirst){
        // we need to calculate countIndex and offsetIndex
        lhsTupleData = malloc(MAX_TUPLE_LEN);
        memset(lhsTupleData, 0, MAX_TUPLE_LEN);
        if(leftIn->getNextTuple(lhsTupleData) == QE_EOF){
            // the scan of left table is over
            leftTableisOver = true;
            free(lhsTupleData);
            // std::cout << "[Error] No data in the left table. " << std::endl;
            return -1;
        }

        // copy the tuple to our vector for later use.
        leftTable.emplace_back(lhsTupleData);

        // Find the attribute's offset of this tuple
        lhsOffsetIndex = ceil((double(lhsAttributes.size())/CHAR_BIT));
        for(auto &attr : lhsAttributes){
            int stepLen = 0;
            if(attr.type == TypeVarChar){
                // we need to add the varchar length to the offset
                int varCharLen = 0;
                memcpy(&varCharLen, (char*)lhsTupleData + lhsOffsetIndex, 4);
                stepLen += varCharLen;
            }

            stepLen += 4;

            if(attr.name == condition.lhsAttr){
                // matched
                if(attr.type != joinTargetType){
                    // std::cerr << "[Error] BNL Join with different join value. " << std::endl;
                    return -1;
                }
                // std::cout << "Set the BNL join lhs. " << std::endl;
                break;
            }

            lhsOffsetIndex += stepLen;
        }

        // get the joinValue and load this tuple.
        if(joinTargetType == TypeInt){
            int joinVal = 0;
            memcpy(&joinVal, (char*)lhsTupleData + lhsOffsetIndex, sizeof(int));
            intBlockBuffer.insert(std::pair<int, void*>(joinVal, lhsTupleData));
        }
        else if(joinTargetType == TypeReal){
            float joinVal;
            memcpy(&joinVal, (char*)lhsTupleData + lhsOffsetIndex, sizeof(float));
            floatBlockBuffer.insert(std::pair<float, void*>(joinVal, lhsTupleData));
        }
        else{
            // varchar
            int varcharLen = 0;
            char* temp;
            memcpy(&varcharLen, (char*)lhsTupleData + lhsOffsetIndex, 4);
            memcpy(&temp, (char*)lhsTupleData + lhsOffsetIndex + 4, varcharLen);
            std::string joinVal(temp);
            varcharBlockBuffer.insert(std::pair<std::string, void*>(joinVal, lhsTupleData));
        }

        totalTupleLen += getLengthOfData(lhsAttributes, lhsTupleData);
    }

    while(totalTupleLen <= numPageinBlock*PAGE_SIZE){
        // clear the tuple to getNext
        lhsTupleData = malloc(MAX_TUPLE_LEN);
        memset(lhsTupleData, 0, MAX_TUPLE_LEN);

        if(!restart){
            if(leftIn->getNextTuple(lhsTupleData) == QE_EOF){
                // the scan of left table is over
                leftTableisOver = true;
                // free(lhsTupleData);
                // std::cout << "[Warning] Scan reaches the end of left table in BNLJoin for the first round. " << std::endl;
                break;
            }

            // copy the tuple to our vector for later use.
            leftTable.emplace_back(lhsTupleData);
        }
        else{
            if(leftTable.empty()){
                // std::cerr << "Invalid leftTable vector. " << std::endl;
                return -1;
            }

            if(it == leftTable.end()){
                leftTableisOver = true;
                // free(lhsTupleData);
                // std::cout << "[Warning] 2  ... Scan reaches the end of left table in BNLJoin. " << std::endl;
                break;
            }

            memcpy(lhsTupleData, *it, MAX_TUPLE_LEN);
            it++;
        }

        // load this tuple.
        if(joinTargetType == TypeInt){
            int joinVal = 0;
            memcpy(&joinVal, (char*)lhsTupleData + lhsOffsetIndex, sizeof(int));
            intBlockBuffer.insert(std::pair<int, void*>(joinVal, lhsTupleData));
        }
        else if(joinTargetType == TypeReal){
            float joinVal;
            memcpy(&joinVal, (char*)lhsTupleData + lhsOffsetIndex, sizeof(float));
            floatBlockBuffer.insert(std::pair<float, void*>(joinVal, lhsTupleData));
        }
        else{
            // varchar
            int varcharLen = 0;
            char* temp;
            memcpy(&varcharLen, (char*)lhsTupleData + lhsOffsetIndex, 4);
            memcpy(&temp, (char*)lhsTupleData + lhsOffsetIndex + 4, varcharLen);
            std::string joinVal(temp);
            varcharBlockBuffer.insert(std::pair<std::string, void*>(joinVal, lhsTupleData));
        }

        totalTupleLen += getLengthOfData(lhsAttributes, lhsTupleData);
    }

    // initialize the currentPos ptr
    if(joinTargetType == TypeInt){
        intCurrentPos = intBlockBuffer.begin();
    }
    else if(joinTargetType == TypeReal){
        floatCurrentPos = floatBlockBuffer.begin();
    }
    else{
        // varchar
        varcharCurrentPos = varcharBlockBuffer.begin();
    }

    // free(lhsTupleData);

    return 0;
}

RC BNLJoin::cleanBlockBuffer() {

    if(joinTargetType == TypeInt){
        if(intBlockBuffer.empty())
            return 0;

        intBlockBuffer.clear();
    }
    else if(joinTargetType == TypeReal){
        if(floatBlockBuffer.empty())
            return 0;

        floatBlockBuffer.clear();
    }
    else{
        // varchar
        if(varcharBlockBuffer.empty())
            return 0;

        varcharBlockBuffer.clear();
    }

    return 0;
}

bool BNLJoin::isBNLJoinSatisfied() {
    // check status
    if(joinTargetType == TypeInt){
        // first, we need to check whether the hashmap is empty or full
        if(intCurrentPos == intBlockBuffer.end())
            return false;

        int rhsJoinVal = 0;
        memcpy(&rhsJoinVal, (char*)rhsTupleData + rhsOffsetIndex, sizeof(int));
        return intCurrentPos->first == rhsJoinVal;
    }
    else if(joinTargetType == TypeReal){
        // first, we need to check whether the hashmap is empty or full
        if(floatCurrentPos == floatBlockBuffer.end())
            return false;

        float rhsJoinVal;
        memcpy(&rhsJoinVal, (char*)rhsTupleData + rhsOffsetIndex, sizeof(float));
        return floatCurrentPos->first == rhsJoinVal;
    }
    else{
        // first, we need to check whether the hashmap is empty or full
        if(varcharCurrentPos == varcharBlockBuffer.end())
            return false;

        char* temp;
        int varCharLen = 0;
        memcpy(&varCharLen, (char*)rhsTupleData + rhsOffsetIndex, 4);
        memcpy(&temp, (char*)rhsTupleData + rhsOffsetIndex + 4, varCharLen);
        std::string rhsJoinVal(temp);

        return varcharCurrentPos->first == rhsJoinVal;
    }
}

/*
 *
 * INLJoin
 *
 */

INLJoin::INLJoin(Iterator *leftIn, IndexScan *rightIn, const Condition &condition) {
    this->leftIn = leftIn;
    this->rightIn = rightIn;
    this->condition = condition;

    this->leftIn->getAttributes(lhsAttributes);
    this->rightIn->getAttributes(rhsAttributes);

    lhsTupleData = malloc(MAX_TUPLE_LEN);
    rhsTupleData = malloc(MAX_TUPLE_LEN);
    lhsSelData = malloc(MAX_TUPLE_LEN);
    rhsSelData = malloc(MAX_TUPLE_LEN);
    joinKey = malloc(MAX_TUPLE_LEN);
    
    this->leftIn->getAttributes(this->lhsAttributes);
    this->rightIn->getAttributes(this->rhsAttributes);
    
    this->lhsSelAttrNames.push_back(this->condition.lhsAttr);
    this->rhsSelAttrNames.push_back(this->condition.rhsAttr);
    
    this->lhsNullSize = ceil(double(this->lhsSelAttrNames.size())/CHAR_BIT);
    this->rhsNullSize = ceil(double(this->rhsSelAttrNames.size())/CHAR_BIT);
    
//    std::cout << "Print condition:" << std::endl;
//    std::cout << this->condition.lhsAttr << "; " << this->condition.rhsAttr << std::endl;
    
//    std::cout << "Print leftAttrs: " << std::endl;
    for(Attribute & attr : this->lhsAttributes){
        this->allAttributes.push_back(attr);
//        std::cout << attr.name << std::endl;
    }
//    std::cout << "Print rightAttrs: " << std::endl;
    for(Attribute & attr : this->rhsAttributes){
        this->allAttributes.push_back(attr);
//        std::cout << attr.name << std::endl;
    }
    
    this->newLhsTuple = true;
    this->newRhsScan = true;
}

void INLJoin::getAttributes(std::vector<Attribute> & attrs) const{
    attrs.clear();
    // add lhs and rhs
    for(auto& i : lhsAttributes){
        attrs.emplace_back(i);
    }

    for(auto& j : rhsAttributes){
        attrs.emplace_back(j);
    }
}

RC INLJoin::getNextTuple(void* data) {
    // we need to check Btree scan status and isFirstTime
    
    RC rc;

    // Only consider join for one attributes and equi-join
    while(true){
        if(this->newLhsTuple){
            while(true){
                rc = leftIn->getNextTuple(this->lhsTupleData);
                if(rc == QE_EOF){
//                    std::cout << "[Warning]: INLJoin::getNextTuple -> scan terminate" << std::endl;
                    return QE_EOF;
                }
                else{
//                    std::cout << "Get tuple from Left" << std::endl;
//                    RelationManager::instance().printTuple(this->lhsAttributes, this->lhsTupleData);
                }
                extractFromReturnedData(this->lhsAttributes, this->lhsSelAttrNames, this->lhsTupleData, this->lhsSelData);
                if (((char *) this->lhsSelData)[0] & (unsigned) 1 << (unsigned) 7){
                    // std::cout << "Tuple from Left Hand is null" << std::endl;
                    continue;
                }
                else{
//                    std::cout << "Get tuple from Left" << std::endl;
                    this->newLhsTuple = false;
                    memmove(this->joinKey, (char *)this->lhsSelData + this->lhsNullSize, MAX_TUPLE_LEN-this->lhsNullSize);
                    break;
                }
            }
        }
    
        if(this->newRhsScan){
            rightIn->setIterator(joinKey, joinKey, true, true);
        }
        while(true){
            rc= rightIn->getNextTuple(this->rhsTupleData);
            if(rc == IX_EOF){
                this->newLhsTuple = true;
                this->newRhsScan = true;
                break;
                // leftIn should get a new tuple
            }
            else{
                this->newRhsScan = false;
//                RelationManager::instance().printTuple(this->rhsAttributes, this->rhsTupleData);
                concatenateData(this->allAttributes, this->lhsAttributes, this->rhsAttributes, this->lhsTupleData, this->rhsTupleData, data);
                return 0;
                // concatenate two schema
            }
        }
    }
}

Aggregate::Aggregate(Iterator *input,          // Iterator of input R
        const Attribute &aggAttr,        // The attribute over which we are computing an aggregate
        AggregateOp op            // Aggregate operation
) {
    this->input = input;
    this->aggAttr = aggAttr;
    this->op = op;
    this->input->getAttributes(this->allAttrs);
    this->opDone = false;
    
    if(aggAttr.type != TypeInt && aggAttr.type != TypeReal){
        // std::cout << "[Error]: Aggregate -> aggregate only on INT or REAL value." <<std::endl;
    }
};

/*
 * the aggregate result is float type
 */
RC Aggregate::getNextTuple(void *data){
    
    if(this->opDone){
        return QE_EOF;
    }
    
    void *selData = malloc(MAX_TUPLE_LEN);
    std::vector<std::string> selAttrNames;
    selAttrNames.push_back(this->aggAttr.name);
    void *tuple = malloc(PAGE_SIZE);
    int nullIndicatorSize = ceil(double(selAttrNames.size())/CHAR_BIT);
    int count = 0;
    
    int resultINT = 0;
    float resultREAL = 0;
    if(this->op == MIN){
        resultINT = std::numeric_limits<int>::max();
        resultREAL = std::numeric_limits<float>::max();
    }

    while (input->getNextTuple(tuple) == 0){
        extractFromReturnedData(this->allAttrs, selAttrNames, tuple, selData);
        if(this->aggAttr.type == TypeInt){
            doOpForINT(selData, nullIndicatorSize, resultINT);
        }
        else{
            doOpForREAL(selData, nullIndicatorSize, resultREAL);
        }
        count++;
    }
    
    // set the aggregation done flag
    this->opDone = true;
    
    
    
    // process the result
    
    // final result is float type
    float finalResult;
    if(this->aggAttr.type == TypeInt){
        finalResult = (float)resultINT;
    }
    else if(this->aggAttr.type == TypeReal){
        finalResult = resultREAL;
    }
    else{
        // std::cout << "[Error]: getNextTuple -> aggregate only on INT or REAL value." <<std::endl;
        return -1;
    }
    
    if(this->op == AVG){
        finalResult /= count;
    }
    
    memcpy(data, selData, nullIndicatorSize);
    free(selData);
    free(tuple);
    if(this->op == COUNT)
    {
        float temp = float(count);
        memcpy((char *)data+nullIndicatorSize, &temp, sizeof(float));
        return 0;
    }
    else{
        memcpy((char *)data+nullIndicatorSize, &finalResult, sizeof(float));
        return 0;
    }
};

RC Aggregate::doOpForINT(void *data, int offset, int &resultINT){
    int temp;
    memcpy(&temp, (char *)data+offset, sizeof(int));
    switch (this->op){
        case MIN:
            if(temp < resultINT)
                resultINT = temp;
            break;
        case MAX:
            if(temp > resultINT)
                resultINT = temp;
            break;
        case SUM:
        case AVG:
            resultINT += temp;
            break;
        case COUNT:
        default:
            break;
    }
//    std::cout << "doOpForINT -> tempINT: " << temp << "resultINT: " << resultINT << std::endl;
    
    return 0;
}

RC Aggregate::doOpForREAL(void *data, int offset, float &resultREAL){
    float temp;
    memcpy(&temp, (char *)data+offset, sizeof(int));
    switch (this->op){
        case MIN:
            if(temp < resultREAL)
                resultREAL = temp;
            break;
        case MAX:
            if(temp > resultREAL)
                resultREAL = temp;
            break;
        case SUM:
        case AVG:
            resultREAL += temp;
            break;
        case COUNT:
        default:
            break;
    }
    
//    std::cout << "doOpForREAL -> tempREAL: " << temp << "resultREAL: " << resultREAL << std::endl;
    return 0;
}

// Please name the output attribute as aggregateOp(aggAttr)
// E.g. Relation=rel, attribute=attr, aggregateOp=MAX
// output attrname = "MAX(rel.attr)"
void Aggregate::getAttributes(std::vector<Attribute> &attrs) const {
    attrs.clear();
    Attribute tmpAttr = this->aggAttr;
    std::string tmp;
    
    //MIN = 0, MAX, COUNT, SUM, AVG
    switch (this->op){
        case MIN:{
            tmp = "MIN(";
            break;
        }
        case MAX:{
            tmp = "MAX";
            break;
        }
        case COUNT:{
            tmp="COUNT(";
            break;
        }
        case SUM:{
            tmp="SUM(";
            break;
        }
        case AVG:{
            tmp = "AVG(";
            break;
        }
        default:{
            // std::cout << "[Error]: None-defined op." << std::endl;
            return;
        }
    }
    tmp += this->aggAttr.name;
    tmp += ")";
    tmpAttr.name = tmp;
    attrs.push_back(tmpAttr);
};

bool compLeftRightVal(AttrType attrType, Condition condition, const void *leftData, const void *rightData, int leftOffset, int rightOffset) {
    
    if(attrType == TypeInt){
//        std::cout << "[TypeInt] compare... " << std::endl;
        
        int leftVal, rightVal = 0;
        memcpy(&leftVal, (char*)leftData + leftOffset, sizeof(int));
        memcpy(&rightVal, (char *)rightData + rightOffset, sizeof(int));
        
        switch (condition.op){
            case EQ_OP:{
                return leftVal == rightVal;
            }
            case LT_OP:{
                return leftVal < rightVal;
            }
            case LE_OP:{
                return leftVal <= rightVal;
            }
            case GT_OP:{
                return leftVal > rightVal;
            }
            case GE_OP:{
                return leftVal >= rightVal;
            };
            case NE_OP:{
                return leftVal != rightVal;
            }
            case NO_OP:{
                return true;
            }
            default:
                // std::cerr << "[Error] wrong op !" << std::endl;
                break;
        }
        
        // std::cerr << "[Error] wrong condition.op. TypeInt " << std::endl;
        return -1;
    }
    else if(attrType == TypeReal){
        float leftVal, rightVal = 0;
        memcpy(&leftVal, (char*)leftData + leftOffset, sizeof(float));
        memcpy(&rightVal, (char *)rightData + rightOffset, sizeof(float));
        
        switch (condition.op){
            case EQ_OP:{
                return leftVal == rightVal;
            }
            case LT_OP:{
                return leftVal < rightVal;
            }
            case LE_OP:{
                return leftVal <= rightVal;
            }
            case GT_OP:{
                return leftVal > rightVal;
            }
            case GE_OP:{
                return leftVal >= rightVal;
            };
            case NE_OP:{
                return leftVal != rightVal;
            }
            case NO_OP:{
                return true;
            }
            default:
                // std::cerr << "[Error] wrong op !" << std::endl;
                break;
        }
        
        // std::cerr << "[Error] wrong condition.op. TypeReal" << std::endl;
        return -1;
    }
    else if(attrType == TypeVarChar){
        //    For VARCHAR: use 4 bytes for the length followed by the characters
        bool res;
        int varCharLen;
        memcpy(&varCharLen, (char*)leftData + leftOffset, 4);
        void* leftVal = malloc(varCharLen + 1);
        memcpy(leftVal, (char*)leftData + leftOffset + 4, varCharLen);
        ((char*)leftVal)[varCharLen] = '\0';
        
        memcpy(&varCharLen, (char *)rightData + rightOffset, 4);
        void* rightVal = malloc(varCharLen + 1);
        memcpy(rightVal, (char*)rightData + rightOffset + 4, varCharLen);
        ((char*)rightVal)[varCharLen] = '\0';
        
        switch (condition.op){
            case EQ_OP:{
                res = ( strcmp((char*)leftVal, (char*)rightVal) == 0 );
                free(leftVal);
                free(rightVal);
                return res;
            }
            case LT_OP:{
                res = ( strcmp((char*)leftVal, (char*)rightVal) < 0 );
                free(leftVal);
                free(rightVal);
                return res;
            }
            case LE_OP:{
                res = ( strcmp((char*)leftVal, (char*)rightVal) <= 0 );
                free(leftVal);
                free(rightVal);
                return res;
            }
            case GT_OP:{
                res = ( strcmp((char*)leftVal, (char*)rightVal) > 0 );
                free(leftVal);
                free(rightVal);
                return res;
            }
            case GE_OP:{
                res = ( strcmp((char*)leftVal, (char*)rightVal) >= 0 );
                free(leftVal);
                free(rightVal);
                return res;
            };
            case NE_OP:{
                res = ( strcmp((char*)leftVal, (char*)rightVal) != 0 );
                free(leftVal);
                free(rightVal);
                return res;
            }
            case NO_OP:{
                return true;
            }
            default:
                // std::cerr << "[Error] wrong op !" << std::endl;
                break;
        }
        
        // std::cerr << "[Error] wrong condition.op. TypeVarChar " << std::endl;
        return -1;
    }
    else{
        // std::cerr << "[Error] wrong attrType. " << std::endl;
        return -1;
    }
}

RC extractFromReturnedData(const std::vector<Attribute> &attrs, const std::vector<std::string> &selAttrNames, const void *data, void *selData) {
    
    int offsetData = 0;
    int offsetSelData = 0;
    int indexForAttributes = 0;
    
    int fieldLength = attrs.size();
    int selAttrsLength = selAttrNames.size();
    
    int nullIndicatorForSelAttrsSize = ceil((double(selAttrsLength) / CHAR_BIT));
    auto *nullsIndicatorForSelAttrs = (unsigned char *) malloc(nullIndicatorForSelAttrsSize);
    // must set the memory to 0
    memset(nullsIndicatorForSelAttrs, 0, nullIndicatorForSelAttrsSize);
    offsetSelData += nullIndicatorForSelAttrsSize;
    
    // 3. NullIndicator
    int nullIndicatorSize = ceil((double(fieldLength) / CHAR_BIT));
    auto *nullsIndicator = (unsigned char *) malloc(nullIndicatorSize);
    memcpy((char *) nullsIndicator, (char *) data, nullIndicatorSize);
    offsetData += nullIndicatorSize;
    
    // 4. iterate over attributes
    int nullByteIndex, bitIndex, fieldIndex = 0;
    int varCharLen = 0;
    
    for(nullByteIndex = 0; nullByteIndex < nullIndicatorSize; nullByteIndex++)
    {
        for(bitIndex = 0; bitIndex < 8; bitIndex++)
        {
            fieldIndex = nullByteIndex*8 + bitIndex;
            
            if((fieldIndex == fieldLength) || (indexForAttributes == selAttrsLength))
                break;
            
            // if the corresponding field is null
            // nullsIndicator bit: 1 -> null, first field correspond the leftmost bit
            if(nullsIndicator[nullByteIndex] & (unsigned) 1 << (unsigned) (7-bitIndex) && attrs[fieldIndex].name == selAttrNames[indexForAttributes])
            {
                /*
                 * set the bit corresponding to this field to 1
                 * indexForAttributes / 8 -> byteindex
                 * 7 - indexForAttributes % 8 -> num of shifts for that bit
                 */
                nullsIndicatorForSelAttrs[indexForAttributes/CHAR_BIT] |= ((unsigned char) 1 << (unsigned) (7 - indexForAttributes%CHAR_BIT));
                indexForAttributes++;
                continue;
            }
            if(nullsIndicator[nullByteIndex] & (unsigned) 1 << (unsigned) (7-bitIndex))
            {
                continue;
            }
            switch(attrs[fieldIndex].type)
            {
                case TypeVarChar:
//                    data = int(len) + string(name)
                    if(attrs[fieldIndex].name == selAttrNames[indexForAttributes]) {
                        memcpy(&varCharLen, (char *)data+offsetData, 4);
                        memcpy((char *)selData+offsetSelData, (char *)data+offsetData, 4+varCharLen);
                        indexForAttributes++;
                        offsetSelData += (4+varCharLen);
                    }
                    offsetData += (4+varCharLen);
                    break;
                case TypeInt:
                    if(attrs[fieldIndex].name == selAttrNames[indexForAttributes])
                    {
                        memcpy((char *)selData+offsetSelData, (char *)data+offsetData, 4);
                        offsetSelData += 4;
                        indexForAttributes++;
                    }
                    offsetData += 4;
                    break;
                case TypeReal:
                    if(attrs[fieldIndex].name == selAttrNames[indexForAttributes])
                    {
                        memcpy((char *)selData+offsetSelData, (char *)data+offsetData, 4);
                        offsetSelData += 4;
                        indexForAttributes++;
                    }
                    offsetData += 4;
                    break;
                default:
//                    std::cout << "[Error] None-specofied data type." << std::endl;
                    break;
            }
            
        }
    }
    // write the nullsIndicatorForAttributes back to data
    memcpy(selData, nullsIndicatorForSelAttrs, nullIndicatorForSelAttrsSize);
    free(nullsIndicator);
    free(nullsIndicatorForSelAttrs);
    return 0;
}

RC concatenateData(std::vector<Attribute> allAttributes, std::vector<Attribute> lhsAttributes, std::vector<Attribute> rhsAttributes, void *lhsTupleData, void *rhsTupleData, void *data){
    int nullIndicatorSize = ceil(double(allAttributes.size())/CHAR_BIT);
    int nullIndicatorSizeLeft = ceil(double(lhsAttributes.size())/CHAR_BIT);
    int nullIndicatorSizeRight = ceil(double(rhsAttributes.size())/CHAR_BIT);
    
    auto *nullIndicator = (char *)malloc(nullIndicatorSize);
    memset(nullIndicator, 0, nullIndicatorSize);
    auto *nullIndicatorLeft = (char *)malloc(nullIndicatorSizeLeft);
    auto *nullIndicatorRight = (char *)malloc(nullIndicatorSizeRight);
    memcpy(nullIndicatorLeft, lhsTupleData, nullIndicatorSizeLeft);
    memcpy(nullIndicatorRight, rhsTupleData, nullIndicatorSizeRight);
    
    int lhsTupleLen = getLengthOfData(lhsAttributes, lhsTupleData);
    int rhsTupleLen = getLengthOfData(rhsAttributes, rhsTupleData);
    
//    std::cout << "lhsTupleLen:" << lhsTupleLen << "; " << "rhsTupleLen" << rhsTupleLen << std::endl;
    
//    memcpy(data, lhsTupleData, nullIndicatorSizeLeft);
    int fieldIndex = 0;
    int leftBitIndex = 0;
    int rightBitIndex = 0;
    
    for(int byteIndex = 0; byteIndex < nullIndicatorSize; byteIndex++){
        for(int bitIndex= 0; bitIndex < 8; bitIndex++){
            fieldIndex = byteIndex*8+bitIndex;
            if(fieldIndex < int(lhsAttributes.size())){
                if(nullIndicatorLeft[(leftBitIndex+1)/CHAR_BIT] & (unsigned char) 1 << (unsigned) (7 - leftBitIndex%CHAR_BIT)){
                    nullIndicator[byteIndex] |= ((unsigned char) 1 << (unsigned) (7 - fieldIndex%CHAR_BIT));
                    // std::cout << "Left hand " << leftBitIndex << "'th attribute is null" <<std::endl;
                }
//                else{
//                    std::cout << "Left hand " << leftBitIndex << "'th attribute is not null" <<std::endl;
//                }
                leftBitIndex++;
            }
            else if(fieldIndex < int(lhsAttributes.size() + rhsAttributes.size())){
                if(nullIndicatorRight[(rightBitIndex+1)/CHAR_BIT] & (unsigned char) 1 << (unsigned) (7 - rightBitIndex%CHAR_BIT)){
                    nullIndicator[byteIndex] |= ((unsigned char) 1 << (unsigned) (7 - fieldIndex%CHAR_BIT));
                    // std::cout << "Right hand " << rightBitIndex << "'th attribute is null" <<std::endl;
                }
//                else{
//                    std::cout << "Right hand " << rightBitIndex << "'th attribute is not null" <<std::endl;
//                }
                rightBitIndex++;
            }
            else{
                break;
            }
        }
    }

    memcpy(data, nullIndicator, nullIndicatorSize);
    memcpy((char *)data+nullIndicatorSize, (char *)lhsTupleData+nullIndicatorSizeLeft, lhsTupleLen-nullIndicatorSizeLeft);
    memcpy((char *)data+nullIndicatorSize+lhsTupleLen-nullIndicatorSizeLeft, (char *)rhsTupleData+nullIndicatorSizeRight, rhsTupleLen-nullIndicatorSizeRight);

    free(nullIndicator);
    free(nullIndicatorLeft);
    free(nullIndicatorRight);
    return 0;
}

int getLengthOfData(const std::vector<Attribute> &attrs, const void *data){
    
    // fieldNumber, offset, length, fieldlength, fieldoffset -> 2 Byte -> char16_t
    
    int offset=0;
    int numAttrs = attrs.size();
    int nullIndicatorSize = ceil(double(attrs.size())/CHAR_BIT);
    char *nullIndicator = (char *)malloc(nullIndicatorSize);
    memcpy(nullIndicator, data, nullIndicatorSize);
    
    offset += nullIndicatorSize;
    int fieldIndex;
    for(int nullByteIndex = 0; nullByteIndex < nullIndicatorSize; nullByteIndex++){
        for(int bitIndex = 0; bitIndex < 8; bitIndex++){
            fieldIndex = nullByteIndex*8 + bitIndex;
            if(fieldIndex == numAttrs)
                break;
            // if the corresponding field is null
            // nullsIndicator bit: 1 -> null, first field correspond the leftmost bit
            if(nullIndicator[nullByteIndex] & (unsigned) 1 << (unsigned) (7-bitIndex)){
                continue;
            }
            switch(attrs[fieldIndex].type){
                case TypeVarChar:
//                    data = int(len) + string(name)
                    int varCharLen;
                    memcpy(&varCharLen, (char *)data+offset, sizeof(varCharLen));
                    offset += 4;    // use two byte to store length, use two byte to store offset
                    offset += varCharLen;
                    break;
                case TypeInt:
                    offset += 4;    // fixed-length field, in-line record field
                    break;
                case TypeReal:
                    offset += 4;    // fixed-length field, in-line record field
                    break;
                default:
                     // std::cout << "[Error] None-specified data type." << std::endl;
                     break;
            }
        }
    }
    // recordLength = offset + 2 -> 2 (store number of fields)
    free(nullIndicator);
    return offset;
}