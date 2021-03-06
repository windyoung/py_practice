# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
'''
@Author: windyoung
@Date: 2020-05-21 16:13:34
LastEditTime: 2020-10-22 19:04:40
LastEditors: windyoung
@Description: 
FilePath: \py_practice\py_task\T_OFFER_INST_RECORD_pb2.py
@
'''
# source: T_OFFER_INST_RECORD.proto


from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='T_OFFER_INST_RECORD.proto',
  package='dmcust',
  syntax='proto2',
  serialized_options=None,
  create_key=_descriptor._internal_create_key,
  serialized_pb=b'\n\x19T_OFFER_INST_RECORD.proto\x12\x06\x64mcust\"\xc7\x01\n\x0fOfferInstRecord\x12\x13\n\x0binstance_id\x18\x01 \x02(\x03\x12\x15\n\roffer_inst_id\x18\x02 \x02(\x03\x12\x10\n\x08offer_id\x18\x03 \x02(\x03\x12\x0b\n\x03seq\x18\x04 \x02(\x03\x12\x10\n\x08\x65\x66\x66_date\x18\x05 \x02(\x03\x12\x10\n\x08\x65xp_date\x18\x06 \x01(\x03\x12\x11\n\tstatus_cd\x18\x07 \x02(\t\x12\x17\n\x0f\x62\x65gin_rent_date\x18\x08 \x01(\x03\x12\x19\n\x11prod_inst_id_list\x18\t \x01(\t'
)




_OFFERINSTRECORD = _descriptor.Descriptor(
  name='OfferInstRecord',
  full_name='dmcust.OfferInstRecord',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='instance_id', full_name='dmcust.OfferInstRecord.instance_id', index=0,
      number=1, type=3, cpp_type=2, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='offer_inst_id', full_name='dmcust.OfferInstRecord.offer_inst_id', index=1,
      number=2, type=3, cpp_type=2, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='offer_id', full_name='dmcust.OfferInstRecord.offer_id', index=2,
      number=3, type=3, cpp_type=2, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='seq', full_name='dmcust.OfferInstRecord.seq', index=3,
      number=4, type=3, cpp_type=2, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='eff_date', full_name='dmcust.OfferInstRecord.eff_date', index=4,
      number=5, type=3, cpp_type=2, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='exp_date', full_name='dmcust.OfferInstRecord.exp_date', index=5,
      number=6, type=3, cpp_type=2, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='status_cd', full_name='dmcust.OfferInstRecord.status_cd', index=6,
      number=7, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=b"".decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='begin_rent_date', full_name='dmcust.OfferInstRecord.begin_rent_date', index=7,
      number=8, type=3, cpp_type=2, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='prod_inst_id_list', full_name='dmcust.OfferInstRecord.prod_inst_id_list', index=8,
      number=9, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=b"".decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto2',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=38,
  serialized_end=237,
)

DESCRIPTOR.message_types_by_name['OfferInstRecord'] = _OFFERINSTRECORD
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

OfferInstRecord = _reflection.GeneratedProtocolMessageType('OfferInstRecord', (_message.Message,), {
  'DESCRIPTOR' : _OFFERINSTRECORD,
  '__module__' : 'T_OFFER_INST_RECORD_pb2'
  # @@protoc_insertion_point(class_scope:dmcust.OfferInstRecord)
  })
_sym_db.RegisterMessage(OfferInstRecord)


# @@protoc_insertion_point(module_scope)



## Data Access Class 
