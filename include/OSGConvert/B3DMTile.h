#pragma once

#include <OSGConvert/OSGNode.h>

namespace scially {

	class B3DMTile: public OSGIndexNode, public QEnableSharedFromThis<B3DMTile> {
	public:
		using Ptr = QSharedPointer<B3DMTile>;
	
		// inherit OSGNode
		// property
		virtual QString name() const override { 
			return "B3DMTile"; 
		}
		// end inherit

		// class OSGTile
		B3DMTile() = default;
		virtual ~B3DMTile() = default;

	private:
	
	};
}
