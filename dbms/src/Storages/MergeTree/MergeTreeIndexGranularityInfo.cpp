#include <Storages/MergeTree/MergeTreeIndexGranularityInfo.h>
#include <Storages/MergeTree/MergeTreeData.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace DB
{
std::optional<std::string> MergeTreeIndexGranularityInfo::getMrkExtensionFromFS(const std::string & path_to_part) const
{
    if (fs::exists(path_to_part))
    {
        fs::directory_iterator end;
        for (fs::directory_iterator part_it(path_to_part); part_it != end; ++part_it)
        {
            const auto & ext = (part_it->path()).extension();
            if (ext == getNonAdaptiveMrkExtension() || ext == getAdaptiveMrkExtension())
                return ext;
        }
    }
    return {};
}

MergeTreeIndexGranularityInfo::MergeTreeIndexGranularityInfo(
    const MergeTreeData & storage)
{
    const auto storage_settings = storage.getSettings();
    fixed_index_granularity = storage_settings->index_granularity;
    /// Granularity is fixed
    if (!storage.canUseAdaptiveGranularity())
        setNonAdaptive();
    else
        setAdaptive(storage_settings->index_granularity_bytes);
}


void MergeTreeIndexGranularityInfo::changeGranularityIfRequired(const std::string & path_to_part)
{
    auto mrk_ext = getMrkExtensionFromFS(path_to_part);
    if (mrk_ext && *mrk_ext == getNonAdaptiveMrkExtension())
        setNonAdaptive();
}

void MergeTreeIndexGranularityInfo::setAdaptive(size_t index_granularity_bytes_)
{
    is_adaptive = true;
    mark_size_in_bytes = getAdaptiveMrkSize();
    marks_file_extension = getAdaptiveMrkExtension();
    index_granularity_bytes = index_granularity_bytes_;
}

void MergeTreeIndexGranularityInfo::setNonAdaptive()
{
    is_adaptive = false;
    mark_size_in_bytes = getNonAdaptiveMrkSize();
    marks_file_extension = getNonAdaptiveMrkExtension();
    index_granularity_bytes = 0;
}

}
