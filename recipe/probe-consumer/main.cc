#include <tiledb/tiledb>
#include <cstdint>
#include <iostream>
#include <vector>
int main() {
  try {
    tiledb::Context ctx;
    for (auto fs : {TILEDB_S3, TILEDB_AZURE, TILEDB_GCS})
      if (!ctx.is_supported_fs(fs)) return 1;
    const char* uri = "native_array_roundtrip";
    tiledb::Domain domain(ctx);
    domain.add_dimension(tiledb::Dimension::create<int32_t>(ctx, "index", {1, 4}, 4));
    tiledb::ArraySchema schema(ctx, TILEDB_DENSE);
    schema.set_domain(domain);
    schema.add_attribute(tiledb::Attribute::create<int32_t>(ctx, "values"));
    tiledb::Array::create(uri, schema);
    std::vector<int32_t> values{11, 22, 33, 44};
    {
      tiledb::Array array(ctx, uri, TILEDB_WRITE);
      tiledb::Query query(ctx, array);
      query.set_layout(TILEDB_ROW_MAJOR).set_data_buffer("values", values);
      if (query.submit() != tiledb::Query::Status::COMPLETE) return 2;
      array.close();
    }
    std::vector<int32_t> result(4);
    {
      tiledb::Array array(ctx, uri, TILEDB_READ);
      tiledb::Query query(ctx, array);
      query.set_layout(TILEDB_ROW_MAJOR).set_data_buffer("values", result);
      if (query.submit() != tiledb::Query::Status::COMPLETE) return 3;
      if (query.result_buffer_elements().at("values").second != 4) return 4;
      array.close();
    }
    if (result != values) return 5;
    tiledb::VFS(ctx).remove_dir(uri);
    std::cout << "Dense array roundtrip passed; S3, Azure and GCS enabled" << std::endl;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 6;
  }
}
