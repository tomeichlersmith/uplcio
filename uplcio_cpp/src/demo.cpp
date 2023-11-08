#include "awkward/LayoutBuilder.h"

#include "demo.h"
#include "snapshot.h"

// Defines taken from how-to-use-header-only-layoutbuilder.md
using UserDefinedMap = std::map<std::size_t, std::string>;
template<class... BUILDERS>
using RecordBuilder = awkward::LayoutBuilder::Record<UserDefinedMap, BUILDERS...>;
template<std::size_t field_name, class BUILDER>
using RecordField = awkward::LayoutBuilder::Field<field_name, BUILDER>;
template<class PRIMITIVE, class BUILDER>
using ListOffsetBuilder = awkward::LayoutBuilder::ListOffset<PRIMITIVE, BUILDER>;
template<class PRIMITIVE>
using NumpyBuilder = awkward::LayoutBuilder::Numpy<PRIMITIVE>;
enum Field : std::size_t {
    one, two
};
using MyBuilder = RecordBuilder<
        RecordField<Field::one, NumpyBuilder<double>>,
        RecordField<Field::two, ListOffsetBuilder<int64_t,
                NumpyBuilder<int32_t>>>
>;


/**
 * Create demo array, and return its snapshot
 * @return pyobject of Awkward Array
 */
pybind11::object create_demo_array() {
    UserDefinedMap fields_map({
        {Field::one, "one"},
        {Field::two, "two"}
    });

    RecordBuilder<
            RecordField<Field::one, NumpyBuilder<double>>,
            RecordField<Field::two, ListOffsetBuilder<int64_t,
                    NumpyBuilder<int32_t>>>
    > builder(fields_map);

    auto &one_builder = builder.content<Field::one>();
    auto &two_builder = builder.content<Field::two>();

    one_builder.append(1.1);

    auto &two_subbuilder = two_builder.begin_list();
    two_subbuilder.append(1);
    two_builder.end_list();

    one_builder.append(2.2);

    two_builder.begin_list();
    two_subbuilder.append(1);
    two_subbuilder.append(2);
    two_builder.end_list();

    one_builder.append(3.3);
    size_t data_size = 3;
    int32_t data[3] = {1, 2, 3};

    two_builder.begin_list();
    two_subbuilder.extend(data, data_size);
    two_builder.end_list();

    return snapshot_builder(builder);
}
