set(CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG")

set(PROTO_FILES
        T_ACCNBR_SUBS_RELA_RECORD.proto
        T_IMSI_SUBS_RELA_RECORD.proto
        T_CUST_ATTR_VALUE_RECORD.proto
        T_CHARGING_CONFIG_ITEM_RECORD.proto
        T_FELLOW_NBR_RECORD.proto
        T_OFFER_INST_RECORD.proto
        T_PROD_INST_REL_RECORD.proto
        T_OFFER_OBJ_INST_REL_RECORD.proto
        T_PROD_INST_RECORD.proto
        T_OFFER_OBJ_INST_REL_FUNC_RECORD.proto
        T_SUBS_REL_RECORD.proto
        T_CC_SUBS_RECORD.proto
        T_PROD_INST_FUNC_RECORD.proto
        T_BAL_SHARE_RECORD.proto
        T_OFFER_INST_ATTR_RECORD.proto
        T_PROD_ATTR_RECORD.proto
        T_BLOCK_REASON_RECORD.proto
        T_ACCT_RECORD.proto
        T_CC_ACCT_RECORD.proto
        T_CUST_RECORD.proto
        T_NP_DB_RECORD.proto
        T_CONTRACT_ITEM_RECORD.proto
        T_BC_MEMBER_RECORD.proto
        T_BC_POOL_RECORD.proto
        T_BILL_OFFER_INST_BAL_SHARE_HIS_RECORD.proto
        T_ACCT_ATTR_VALUE_RECORD.proto
        T_GEO_HOME_ZONE_RECORD.proto
)

include_directories(${Protobuf_INCLUDE_DIRS})

protobuf_generate_cpp(PROTO_SRCS  PROTO_HDRS  ${PROTO_FILES})

add_library(dmcust_pb SHARED ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(dmcust_pb ${Protobuf_LIBRARIES})

set(TARGETS_IN_THIS_DIR
        dmcust_pb
        )

INSTALL_MACRO()



